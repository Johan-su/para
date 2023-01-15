namespace TopDownParser
{

static Expr *parse_operator(Lexer *lexer, Usize *index)
{
    BinOperator *boperator = alloc<BinOperator>(1);
    boperator->expr.expr_type = EXPR_BIN_OPERATOR;


    if (is_token(peek(lexer, *index), OPERATOR_PLUS)) boperator->opt = BinOperatorType::PLUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MINUS)) boperator->opt = BinOperatorType::MINUS;
    else if (is_token(peek(lexer, *index), OPERATOR_MULTIPLY)) boperator->opt = BinOperatorType::MULTIPLY;
    else if (is_token(peek(lexer, *index), OPERATOR_DIVIDE)) boperator->opt = BinOperatorType::DIVIDE;
    else
    {
        TODO("handle error expected operator but found none");
    }

    *index += 1;
    return (Expr *)boperator;
}



static Expr *parse_number(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    if (is_token(peek(lexer, *index), NUMBER))
    {
        if (is_token(peek(lexer, *index + 1), NUMBER))
        {
            TODO("unexpected number after number");
        }
        Number *number = alloc<Number>(1);
        number->val = atof(lexer->tokens[*index].str_val);
        number->expr.expr_type = EXPR_NUMBER;
        root = (Expr *)number;
        *index += 1;
    }

    return root;
}

// <Expr> := <Expr> <BinOperator> <Expr> | (<Expr>) | <Number>


// https://www.tutorialspoint.com/what-is-left-recursion-and-how-it-is-eliminated

// not left recursion
// <Expr> := (<Expr>) <Expr'> | <Number> <Expr'>
// <Expr'> := {<BinOperator> <Expr> <Expr'>}*

static Expr *parse_expr(Lexer *lexer, Usize *index);
static Expr *parse_alt_expr(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    if (is_token(peek(lexer, *index), END_TOKEN))
    {
        return nullptr;
    }
    if (is_operator_token(peek(lexer, *index)))
    {
        Expr *op = parse_operator(lexer, index);
        Expr *expr = parse_expr(lexer, index);
        Expr *alt_expr = parse_alt_expr(lexer, index); 


        op->right = expr;

        if (alt_expr != nullptr)
        {
            assert(alt_expr->expr_type == EXPR_BIN_OPERATOR);
            alt_expr->left = op;
            root = alt_expr;
        }
        else
        {
            root = op;
        }
    }


    return root;
}

static Expr *parse_expr(Lexer *lexer, Usize *index)
{
    Expr *root = nullptr;

    assert(lexer->tokens[*index].token_type != INVALID_TOKEN);

    if (is_token(peek(lexer, *index), END_TOKEN))
    {
        return nullptr;
    }
    else if (is_token(peek(lexer, *index), OPEN_PARENTHESIS))
    {
        *index += 1;
        Expr *expr = parse_expr(lexer, index);
        if (!is_token(peek(lexer, *index), CLOSE_PARENTHESIS))
        {
            TODO("handle error right parens must match with left parens");
        }
        *index += 1;
        Expr *alt_expr = parse_alt_expr(lexer, index);
        if (alt_expr != nullptr)
        {
            alt_expr->left = expr;
            root = alt_expr;
        }
        else
        {
            root = expr;
        }
    }
    else if (is_token(peek(lexer, *index), NUMBER))
    {
        Expr *number = parse_number(lexer, index);

        Expr *alt_expr = parse_alt_expr(lexer, index);

        if (alt_expr != nullptr)
        {
            alt_expr->left = number;
            root = alt_expr;
        }
        else
        {
            root = number;
        }

    }
    else
    {
        assert(false && "unreachable");
    }

    return root;
}

} // namespace TopDownParser






namespace BottomUpParser
{



/*

    1:
    <S>     := . <Expr> $
    <Expr>  := . (<Expr>) <Expr'>
    <Expr>  := . <Number> <Expr'>
    <Expr'> := . <BinOperator> <Expr> <Expr'>
    <Expr'> := . <>


*/






// https://boxbase.org/entries/2019/oct/14/lr1-parsing-tables/
// https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf

#define PARSE_STACK_CAP 4096

struct ExprOrToken
{
    bool is_token;

    union
    {
        Expr *e;
        Token *t;
    };
};  


struct ParseStack
{
    ExprOrToken data[PARSE_STACK_CAP];
    Usize index;
};





static ExprOrToken pop(ParseStack *s)
{
    if (s->index >= PARSE_STACK_CAP)
    {
        TODO("handle error tried to pop from empty stack");
    }
    return s->data[s->index++];
}

static void push(ParseStack *s, ExprOrToken t)
{
    if (s->index <= 0)
    {
        TODO("handle error push to full stack");
    }
    s->data[--s->index] = t;
}



static Usize g_token_index = 0;


static void next_token(Lexer *lexer)
{
    if (g_token_index < lexer->count)
    {
        g_token_index += 1;
    }
}

static Token *peek(Lexer *lexer)
{
    return &lexer->tokens[g_token_index];
}



//static action[][]


static void shift(ParseStack *stack, Lexer *lexer, U64 k)
{
    Token *t = peek(lexer);
    next_token(lexer);
    ExprOrToken eot = {
        .is_token = true,
        .t = t,
    };
    push(stack, eot);
}


static void reduce()
{

}


static Expr *parse_expr(Lexer *lexer)
{
    ParseStack g_stack = {
        .data = {},
        .index = PARSE_STACK_CAP,
    };

    return nullptr;
}















} // namespace BottomUpParser
 