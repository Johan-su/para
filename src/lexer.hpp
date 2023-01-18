#ifndef LEXER_HPP
#define LEXER_HPP

enum class TokenType
{
    INVALID_TOKEN = 0,
    OPERATOR_PLUS,    
    OPERATOR_MINUS,    
    OPERATOR_MULTIPLY,    
    OPERATOR_DIVIDE,

    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,

    NUMBER,

    IDENTIFIER,
    EQUAL,

    END_TOKEN,

    TOKEN_COUNT
};


struct Token
{
    const char *str_val;
    Usize str_count;
    TokenType token_type;
};


#define MAX_TOKEN_CAP 65535


struct Lexer
{
    Token tokens[MAX_TOKEN_CAP];
    Usize count;
};

static void push_token(Lexer *lexer, Token token);
static bool is_digit(char n);
static bool is_whitespace(char n);
static bool is_letter(char n);
static void tokenize(Lexer *lexer, const char *source);
static const char *tokentype_to_str(TokenType tt);
static void print_tokens(Lexer *lexer);

#endif
// ---------------------
#ifdef LEXER_HPP_IMPLEMENTATION
#undef LEXER_HPP_IMPLEMENTATION

static Lexer g_lexer = {
    .tokens = {},
    .count = 0,
};



static void push_token(Lexer *lexer, Token token)
{
    if (lexer->count >= MAX_TOKEN_CAP)
    {
        TODO("handle ERROR more tokens than token cap");
    }

    lexer->tokens[lexer->count] = token;
    lexer->count += 1;
}



static bool is_digit(char n)
{
    if (n >= '0' && n <= '9')
    {
        return true;
    }
    return false;
}

static bool is_whitespace(char n)
{
    switch (n)
    {
        case ' ': return true;
        case '\n': return true;
    }
    return false;
}

static bool is_letter(char n)
{
    return (n >= 'A' && n <= 'Z') || (n >= 'a' && n <= 'z');
}


static void tokenize(Lexer *lexer, const char *source)
{
    for (Usize i = 0; source[i] != '\0'; ++i)
    {
             if (source[i] == '(') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPEN_PARENTHESIS});
        else if (source[i] == ')') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::CLOSE_PARENTHESIS});
        else if (source[i] == '+') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_PLUS});
        else if (source[i] == '-') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_MINUS});
        else if (source[i] == '*') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_MULTIPLY});
        else if (source[i] == '/') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::OPERATOR_DIVIDE});
        else if (source[i] == '=') push_token(lexer, Token {.str_val = &source[i], .str_count = 1, .token_type = TokenType::EQUAL});
        else if (is_digit(source[i]))
        {
            Token number_token = {};
            {
                number_token.str_val = &source[i];
                number_token.token_type = TokenType::NUMBER;

                Usize count = 1;
                while (is_digit(source[i + count]))
                {
                    count += 1;
                }
                if (source[i + count] == '.')
                {
                    count += 1;
                    while (is_digit(source[i + count]))
                    {
                        count += 1;
                    }
                }
                number_token.str_count = count;
                i += count - 1;
            }
            push_token(lexer, number_token);
        }
        else if (is_letter(source[i]))
        {
            Token id_token = {};
            id_token.str_val = &source[i];
            id_token.token_type = TokenType::IDENTIFIER;

            Usize count = 1;
            while (is_letter(source[i + count]))
            {
                count += 1;
            }
            id_token.str_count = count;
            i += count - 1;

            push_token(lexer, id_token);
        }
        else if (is_whitespace(source[i])) /*do nothing*/;
        else
        {
            fprintf(stderr, "ERROR: unhandled character %c\n", source[i]);
            assert(false && "unhandled character");
        }
    }
    push_token(lexer, Token {.str_val = nullptr, .str_count = 0, .token_type = TokenType::END_TOKEN});
}



static const char *tokentype_to_str(TokenType tt)
{
    switch (tt)
    {
        case TokenType::INVALID_TOKEN: return "INVALID_TOKEN";
        case TokenType::OPERATOR_PLUS: return "OPERATOR_PLUS";    
        case TokenType::OPERATOR_MINUS: return "OPERATOR_MINUS";    
        case TokenType::OPERATOR_MULTIPLY: return "OPERATOR_MULTIPLY";    
        case TokenType::OPERATOR_DIVIDE: return "OPERATOR_DIVIDE";
        case TokenType::OPEN_PARENTHESIS: return "OPEN_PARENTHESIS";
        case TokenType::CLOSE_PARENTHESIS: return "CLOSE_PARENTHESIS";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::END_TOKEN: return "END_TOKEN";
        case TokenType::TOKEN_COUNT: return "TOKEN_COUNT"; 
    }
    
    assert(false && "no string representation of token type found");
    return nullptr;
}




static void print_tokens(Lexer *lexer)
{
    for (Usize i = 0; i < lexer->count; ++i)
    {
        Token *token = &lexer->tokens[i];
        printf("%s VALUE: %.*s\n", tokentype_to_str(token->token_type), (int)token->str_count, token->str_val);
    }
}
#endif