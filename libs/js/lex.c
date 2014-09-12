#include "js/js.h"
#include "unicode/type.h"
#include "unicode/hash.h"

#include "c/stdlib.h"
#include "c/stdio.h"
#include "c/assert.h"
#include "c/stdbool.h"

enum {
    TAB = 0x9,
    VT = 0xB,
    FF = 0xC,
    SP = 0x20,
    NBSP = 0xA0,
    BOM = 0xFEFF,

    LF = 0xA,
    CR = 0xD,
    LS = 0x2028,
    PS = 0x2029,

    ZWNJ = 0x200C,
    ZWJ = 0x200D
};

static hashmap_t *keywords = NULL;

static void initKeyword(void) {
    keywords = hashmap_new_utf16(97);
    static struct {
        char *name;
        uint16_t value;
    } map[] = {
        {"break", BREAK},
        {"case", CASE},
        {"catch", CATCH},
        {"continue", CONTINUE},
        {"debugger", DEBUGGER},
        {"default", DEFAULT},
        {"delete", DELETE},
        {"do", DO},
        {"else", ELSE},
        {"finally", FINALLY},
        {"for", FOR},
        {"function", FUNCTION},
        {"if", IF},
        {"in", IN},
        {"instanceof", INSTANCEOF},
        {"new", NEW},
        {"return", RETURN},
        {"switch", SWITCH},
        {"this", THIS},
        {"throw", THROW},
        {"try", TRY},
        {"typeof", TYPEOF},
        {"var", VAR},
        {"void", VOID},
        {"while", WHILE},
        {"with", WITH},

        {"class", RESERVED_WORD},
        {"const", RESERVED_WORD},
        {"enum", RESERVED_WORD},
        {"export", RESERVED_WORD},
        {"extends", RESERVED_WORD},
        {"import", RESERVED_WORD},
        {"super", RESERVED_WORD},

        {"implements", RESERVED_STRICT},
        {"interface", RESERVED_STRICT},
        {"let", RESERVED_STRICT},
        {"package", RESERVED_STRICT},
        {"private", RESERVED_STRICT},
        {"protected", RESERVED_STRICT},
        {"public", RESERVED_STRICT},
        {"static", RESERVED_STRICT},
        {"yield", RESERVED_STRICT},

        {"null", NULL_LIT},
        {"true", TRUE_LIT},
        {"false", FALSE_LIT}
    };
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        utf16_string_t *utf16 = malloc(sizeof(utf16_string_t));
        *utf16 = unicode_toUtf16(UTF8_STRING(map[i].name));
        hashmap_put(keywords, utf16, (void *)(size_t)map[i].value);
    }
}

static uint16_t lookupKeyword(utf16_string_t kwd) {
    if (!keywords) {
        initKeyword();
    }
    return (size_t)hashmap_get(keywords, &kwd);
}

static uint16_t lookahead(lex_t *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr];
}

static uint16_t next(lex_t *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr++];
}

static void createBuffer(lex_t *lex) {
    lex->data.length = 0;
    lex->data.size = 10;
    lex->data.buffer = malloc(10 * sizeof(uint16_t));
}

static void appendToBuffer(lex_t *lex, uint16_t ch) {
    if (lex->data.length == lex->data.size) {
        lex->data.size *= (lex->data.size * 3) / 2 + 1;
        lex->data.buffer = realloc(lex->data.buffer, lex->data.size * sizeof(uint16_t));
    }
    lex->data.buffer[lex->data.length++] = ch;
}

static utf16_string_t cleanBuffer(lex_t *lex) {
    utf16_string_t ret = {
        .str = lex->data.buffer,
        .len = lex->data.length
    };
    return ret;
}

static token_t *createToken(uint16_t type) {
    token_t *token = malloc(sizeof(struct struct_token));
    token->type = type;
    return token;
}

void lex_disposeToken(token_t *tk) {
    if (tk->type == ID || tk->type == STR) {
        if (tk->stringValue.str) {
            free(tk->stringValue.str);
        }
    }
    free(tk);
}

static token_t *stateDefault(lex_t *lex);
static token_t *stateSingleLineComment(lex_t *lex);
static token_t *stateMultiLineComment(lex_t *lex);
static token_t *stateIdentiferPart(lex_t *lex);
static token_t *stateHexIntegerLiteral(lex_t *lex);
static token_t *stateOctIntegerLiteral(lex_t *lex);
static token_t *stateDoubleString(lex_t *lex);
static token_t *stateSingleString(lex_t *lex);

static token_t *stateDefault(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case TAB:
        case VT:
        case FF:
        case SP:
        case NBSP:
        case BOM: {
            return NULL;
        }
        case CR: {
            if (lex->lookahead(lex) == LF) {
                lex->next(lex);
            }
        }
        case LF:
        case LS:
        case PS: {
            lex->lineBefore = true;
            return NULL;
        }
        case '/': {
            uint16_t next = lex->lookahead(lex);
            if (next == '/') {
                lex->state = stateSingleLineComment;
            } else if (next == '*') {
                lex->state = stateMultiLineComment;
            } else {
                if (lex->regexp) {
                    //TODO
                    assert(!"Regexp is not currently supported");
                } else {
                    if (lex->lookahead(lex) == '=') {
                        lex->next(lex);
                        return createToken(DIV_ASSIGN);
                    } else {
                        return createToken(DIV);
                    }
                }
            }
            return NULL;
        }
        case '$':
        case '_': {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return NULL;
        }
        case '\\':
            //TODO Unicode Escape Sequence
            assert(0);
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
        case '.':
        case ';':
        case ',':
        case '~':
        case '?':
        case ':': {
            return createToken(next);
        }
        case '<': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return createToken(LTEQ);
            } else if (nch == '<') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    return createToken(SHL_ASSIGN);
                } else {
                    return createToken(SHL);
                }
            } else {
                return createToken(LT);
            }
        }
        case '>': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return createToken(GTEQ);
            } else if (nch == '>') {
                lex->next(lex);
                uint16_t n2ch = lex->lookahead(lex);
                if (n2ch == '=') {
                    lex->next(lex);
                    return createToken(SHR_ASSIGN);
                } else if (n2ch == '>') {
                    lex->next(lex);
                    if (lex->lookahead(lex) == '=') {
                        lex->next(lex);
                        return createToken(USHR_ASSIGN);
                    } else {
                        return createToken(USHR);
                    }
                } else {
                    return createToken(SHR);
                }
            } else {
                return createToken(GT);
            }
        }
        case '=':
        case '!': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    return createToken(next == '=' ? FULL_EQ : FULL_INEQ);
                } else {
                    return createToken(next | ASSIGN_FLAG);
                }
            } else {
                return createToken(next);
            }
        }
        case '+':
        case '-':
        case '&':
        case '|': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return createToken(next | ASSIGN_FLAG);
            } else if (nch == next) {
                lex->next(lex);
                return createToken(next | DOUBLE_FLAG);
            } else {
                return createToken(next);
            }
        }
        case '*':
        case '%':
        case '^': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                return createToken(next | ASSIGN_FLAG);
            } else {
                return createToken(next);
            }
        }
        case '0': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == 'x' || nch == 'X') {
                lex->next(lex);
                lex->state = stateHexIntegerLiteral;
                lex->data.number = 0;
                return NULL;
            } else if (nch >= '0' && nch <= '9') {
                if (lex->strictMode) {
                    assert(!"Syntax Error: Octal literals are not allowed in strict mode.");
                } else {
                    lex->state = stateOctIntegerLiteral;
                    lex->data.number = 0;
                }
                return NULL;
            }
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            assert(!"DEC not supported yet");
        }
        case '"': {
            lex->state = stateDoubleString;
            createBuffer(lex);
            return NULL;
        }
        case '\'': {
            lex->state = stateSingleString;
            createBuffer(lex);
            return NULL;
        }
        case 0xFFFF: {
            lex->lineBefore = true;
            return createToken(END_OF_FILE);
        }
    }

    switch (unicode_getType(next)) {
        case SPACE_SEPARATOR: {
            return NULL;
        }
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER: {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return NULL;
        }
    }
    assert(0);
    return NULL;
}

static token_t *stateSingleLineComment(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->state = stateDefault;
            return NULL;
        }
    }
    lex->next(lex);
    return NULL;
}

static token_t *stateMultiLineComment(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '*': {
            if (lex->lookahead(lex) == '/') {
                lex->next(lex);
                lex->state = stateDefault;
            }
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->lineBefore = true;
        }
    }
    return NULL;
}

static token_t *stateIdentiferPart(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case '$':
        case '_':
        case ZWNJ:
        case ZWJ: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return NULL;
        }
        case '\\': {
            //TODO Unicode Escape Sequence
            assert(0);
        }
    }
    switch (unicode_getType(next)) {
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER:
        case CONNECTOR_PUNCTUATION:
        case DECIMAL_DIGIT_NUMBER:
        case NON_SPACING_MARK:
        case COMBINING_SPACING_MARK: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return NULL;
        }
    }
    lex->state = stateDefault;

    utf16_string_t str = cleanBuffer(lex);

    if (!lex->parseId) {
        token_t *token = createToken(ID);
        token->stringValue = str;
        return token;
    }

    uint16_t type = lookupKeyword(str);

    if (type == RESERVED_STRICT) {
        if (lex->strictMode) {
            type = RESERVED_WORD;
        } else {
            type = 0;
        }
    }

    if (!type) {
        token_t *token = createToken(ID);
        token->stringValue = str;
        return token;
    } else if (type == RESERVED_WORD) {
        assert(!"SyntaxError: Unexpected reserved word.");
        return NULL;
    } else {
        free(str.str);
        return createToken(type);
    }
}

static token_t *stateOctIntegerLiteral(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '7') {
        lex->next(lex);
        lex->data.number = lex->data.number * 8 + (next - '0');
    }  else {
        if (next == '8' || next == '9' || next == '$' || next == '_' || next == '\\') {
            assert(!"SyntaxError: Unexpected character after number literal.");
        }
        switch (unicode_getType(next)) {
            case UPPERCASE_LETTER:
            case LOWERCASE_LETTER:
            case TITLECASE_LETTER:
            case MODIFIER_LETTER:
            case OTHER_LETTER:
            case LETTER_NUMBER:
                assert(!"SyntaxError: Unexpected character after number literal.");
        }

        lex->state = stateDefault;

        token_t *token = createToken(NUM);
        token->numberValue = lex->data.number;

        return token;
    }
    return NULL;
}

static token_t *stateHexIntegerLiteral(lex_t *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16 + (next - '0');
    } else if (next >= 'a' && next <= 'f') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16 + 10 + (next - 'a');
    } else if (next >= 'A' && next <= 'F') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16 + 10 + (next - 'a');
    } else {
        if (next == '$' || next == '_' || next == '\\') {
            assert(!"SyntaxError: Unexpected character after number literal.");
        }
        switch (unicode_getType(next)) {
            case UPPERCASE_LETTER:
            case LOWERCASE_LETTER:
            case TITLECASE_LETTER:
            case MODIFIER_LETTER:
            case OTHER_LETTER:
            case LETTER_NUMBER:
                assert(!"SyntaxError: Unexpected character after number literal.");
        }

        lex->state = stateDefault;

        token_t *token = createToken(NUM);
        token->numberValue = lex->data.number;

        return token;
    }
    return NULL;
}

static void dealEscapeSequence(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case CR: {
            if (lex->lookahead(lex) == LF) {
                lex->next(lex);
                return;
            }
        }
        case LF:
        case LS:
        case PS:
            return;
        default:
            assert(0);
    }
}

static token_t *stateDoubleString(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '"': {
            lex->state = stateDefault;

            token_t *token = createToken(STR);
            token->stringValue = cleanBuffer(lex);

            return token;
        }
        case '\\': {
            dealEscapeSequence(lex);
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: String literal is not enclosed.");
        default: {
            appendToBuffer(lex, next);
            return NULL;
        }
    }
}

static token_t *stateSingleString(lex_t *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '\'': {
            lex->state = stateDefault;

            token_t *token = createToken(STR);
            token->stringValue = cleanBuffer(lex);

            return token;
        }
        case '\\': {
            dealEscapeSequence(lex);
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: String literal is not enclosed.");
        default: {
            appendToBuffer(lex, next);
            return NULL;
        }
    }
}

lex_t *lex_new(char *chr) {
    lex_t *l = malloc(sizeof(struct struct_lex));
    l->next = next;
    l->lookahead = lookahead;
    l->state = stateDefault;
    l->content = unicode_toUtf16(UTF8_STRING(chr));
    l->ptr = 0;
    l->regexp = false;
    l->strictMode = true;
    l->lineBefore = false;
    l->parseId = true;
    return l;
}

token_t *lex_next(lex_t *lex) {
    while (1) {
        token_t *ret = lex->state(lex);
        if (ret) {
            ret->lineBefore = lex->lineBefore;
            lex->lineBefore = false;
            return ret;
        }
    }
}

void examine_token(token_t *ret) {
    if (ret->lineBefore) {
        printf("(line)");
    }
    if (ret->type < ASSIGN_FLAG) {
        printf("(%c)", ret->type);
    } else if (ret->type < DOUBLE_FLAG) {
        printf("(%c=)", ret->type & ~ ASSIGN_FLAG);
    } else if (ret->type < OTHER_FLAG) {
        printf("(%c=)", ret->type & ~ DOUBLE_FLAG);
    } else {
        switch (ret->type) {
            case FULL_EQ: {
                printf("(===)");
                break;
            }
            case FULL_INEQ: {
                printf("(!==)");
                break;
            }
            case SHL: {
                printf("(<<)");
                break;
            }
            case SHR: {
                printf("(>>)");
                break;
            }
            case USHR: {
                printf("(>>>)");
                break;
            }
            case SHL_ASSIGN: {
                printf("(<<=)");
                break;
            }
            case SHR_ASSIGN: {
                printf("(>>=)");
                break;
            }
            case USHR_ASSIGN: {
                printf("(>>>=)");
                break;
            }
            case STR: {
                utf8_string_t str = unicode_toUtf8(ret->stringValue);
                printf("(STR, %.*s)", str.len, str.str);
                break;
            }
            case ID: {
                utf8_string_t str = unicode_toUtf8(ret->stringValue);
                printf("(ID, %.*s)", str.len, str.str);
                break;
            }
            case NUM: {
                printf("(NUM, %d)", (size_t)ret->numberValue);
                break;
            }
            case BREAK: printf("(break"); break;
            case CASE: printf("(case)"); break;
            case CATCH: printf("(catch)"); break;
            case CONTINUE: printf("(continue)"); break;
            case DEBUGGER: printf("(debugger)"); break;
            case DEFAULT: printf("(default)"); break;
            case DELETE: printf("(delete)"); break;
            case DO: printf("(do)"); break;
            case ELSE: printf("(else)"); break;
            case FINALLY: printf("(finally)"); break;
            case FOR: printf("(for)"); break;
            case FUNCTION: printf("(function)"); break;
            case IF: printf("(if)"); break;
            case IN: printf("(in)"); break;
            case INSTANCEOF: printf("(instanceof)"); break;
            case NEW: printf("(new)"); break;
            case RETURN: printf("(return)"); break;
            case SWITCH: printf("(switch)"); break;
            case THIS: printf("(this)"); break;
            case THROW: printf("(throw)"); break;
            case TRY: printf("(try)"); break;
            case TYPEOF: printf("(typeof)"); break;
            case VAR: printf("(var)"); break;
            case VOID: printf("(void)"); break;
            case WHILE: printf("(while)"); break;
            case WITH: printf("(with)"); break;
            default:
                assert(0);
        }
    }
}
