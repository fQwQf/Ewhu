#include "evaluator.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

std::shared_ptr<Object> Evaluator::eval_eval(const std::string &line, Scope &scp)
{
    Lexer lexer;
    Parser parser;
    Evaluator evaluator;

    lexer.source = line;
    std::vector<Token> new_tokens = lexer.scanTokens();

    if ((lexer.braceStatus == 0) &&
        ((--new_tokens.end())->type == TokenType::SEMICOLON ||
         (--new_tokens.end())->type == TokenType::RIGHT_BRACE))
    {
        parser.new_sentence(new_tokens.begin(), new_tokens.end());
        parser.parse_program();
        new_tokens.clear();
        auto program = parser.m_program;
        return evaluator.eval(program, scp);
    }
    return nullptr;
}

std::shared_ptr<Object> Evaluator::eval_function(const std::shared_ptr<FunctionIdentifier> &node, Scope &scp)
{
    auto it = scp.m_func.find(node->m_name);
    if (it == scp.m_func.end())
    {
        if (node->m_name == "print")
        {
            std::cout << eval(node->m_initial_list[0], scp)->str() << std::endl;
            return std::make_shared<Ob_Null>(Ob_Null());
        }
        if (node->m_name == "eval")
        {
            return eval_eval(eval(node->m_initial_list[0], scp)->str(), scp);
        }
        return new_error("Evaluator::eval_function: function %s not found", node->m_name.c_str());
    }
    auto function = it->second;
    Scope temp_scp(scp.m_var, scp.m_func);
    if (function->m_initial_list.size() != node->m_initial_list.size())
    {
        return new_error("Evaluator::eval_function: function %s arguments not match", node->m_name.c_str());
    }

    for (int i = 0; i < function->m_initial_list.size(); i++)
    {
        eval_assign_expression(function->m_initial_list[i]->m_name, eval(node->m_initial_list[i], scp), temp_scp);
    }
    auto result = eval_statement_block(it->second->m_statement->m_statements, temp_scp);
    if (result->type() == Object::OBJECT_RETURN)
        return std::dynamic_pointer_cast<Ob_Return>(result)->m_expression;
    return std::make_shared<Ob_Null>();
}

std::shared_ptr<Object> Evaluator::eval_assign_expression(const std::string &name, const std::shared_ptr<Object> &value, Scope &scp)
{
    auto it = scp.m_var.find(name);
    if (it == scp.m_var.end())
    {
        scp.m_var.insert(std::make_pair(name, value));
    }
    else
    {
        it->second = value;
    }
    return value;
}

std::shared_ptr<Object> Evaluator::eval_prefix(const TokenType &op, const std::shared_ptr<Object> &right)
{
    switch (right->type())
    {
    case Object::OBJECT_INTEGER:
    {
        return eval_integer_prefix_expression(op, right);
    }
    case Object::OBJECT_FRACTION:
    {
        return eval_fraction_prefix_expression(op, right);
    }
    case Object::OBJECT_BOOLEAN:
    {
        return eval_boolean_prefix_expression(op, right);
    }
    default:
        break;
    }
    return new_error("Evaluator: unknown operator: %s %s", "operator", right->name().c_str());
}

std::shared_ptr<Object> Evaluator::eval_integer_prefix_expression(const TokenType &op, const std::shared_ptr<Object> &right)
{
    auto r = std::dynamic_pointer_cast<Ob_Integer>(right);
    if (op == TokenType::PLUS)
    {
        return std::make_shared<Ob_Integer>(r->m_value);
    }
    else if (op == TokenType::MINUS)
    {
        return std::make_shared<Ob_Integer>(-r->m_value);
    }
    else
    {
        return new_error("Evaluator::eval_integer_prefix_expression unknown operator: %s %s", TokenTypeToString[op].c_str(), right->name().c_str());
    }
}

std::shared_ptr<Object> Evaluator::eval_fraction_prefix_expression(const TokenType &op, const std::shared_ptr<Object> &right)
{
    auto r = std::dynamic_pointer_cast<Ob_Fraction>(right);
    if (op == TokenType::PLUS)
    {
        std::shared_ptr<Ob_Fraction> s(new Ob_Fraction(r->num, r->den));
        return s;
    }
    else if (op == TokenType::MINUS)
    {
        std::shared_ptr<Ob_Fraction> s(new Ob_Fraction(-r->num, r->den));
        return s;
    }
    else
    {
        return new_error("Evaluator: unknown operator: %s %s", TokenTypeToString[op].c_str(), right->name().c_str());
    }
}

std::shared_ptr<Object> Evaluator::eval_boolean_prefix_expression(const TokenType &op, const std::shared_ptr<Object> &right)
{
    auto r = std::dynamic_pointer_cast<Ob_Boolean>(right);
    if (op == TokenType::BANG || op == TokenType::MINUS)
    {
        std::shared_ptr<Ob_Boolean> s(new Ob_Boolean(!r->m_value));
        return s;
    }
    else
    {
        return new_error("Evaluator: unknown operator: %s %s", TokenTypeToString[op].c_str(), right->name().c_str());
    }
}

std::shared_ptr<Object> Evaluator::eval_infix(const TokenType op, const std::shared_ptr<Object> &left,
                                              const std::shared_ptr<Object> &right, Scope &scp) // 中缀表达式求值
{
    // std::cout << "eval_infix: " << left->str() << "(" << left->name() << ") " << TokenTypeToString[op]
    //           << " " << right->str() << "(" << right->name() << ")" << std::endl;

    // int(bool) op int(bool)
    if (((left->type() == Object::OBJECT_INTEGER) || (left->type() == Object::OBJECT_BOOLEAN)) &&
        ((right->type() == Object::OBJECT_INTEGER) || (right->type() == Object::OBJECT_BOOLEAN)))
        return eval_integer_infix_expression(op, left, right);

    // fraction op fraction
    if (left->type() == Object::OBJECT_FRACTION && right->type() == Object::OBJECT_FRACTION)
        return eval_fraction_infix_expression(op, left, right);

    // fraction op int
    if (left->type() == Object::OBJECT_FRACTION && right->type() == Object::OBJECT_INTEGER)
        return eval_fraction_infix_expression(op, left, std::make_shared<Ob_Fraction>(std::dynamic_pointer_cast<Ob_Integer>(right)->m_value, 1));

    // int op fraction
    if (left->type() == Object::OBJECT_INTEGER && right->type() == Object::OBJECT_FRACTION)
        return eval_fraction_infix_expression(op, std::make_shared<Ob_Fraction>(std::dynamic_pointer_cast<Ob_Integer>(left)->m_value, 1), right);

    // fraction op bool
    if (left->type() == Object::OBJECT_FRACTION && right->type() == Object::OBJECT_BOOLEAN)
        return eval_fraction_infix_expression(op, left, std::make_shared<Ob_Fraction>(std::dynamic_pointer_cast<Ob_Boolean>(right)->m_value, 1));

    // bool op fraction
    if (left->type() == Object::OBJECT_BOOLEAN && right->type() == Object::OBJECT_FRACTION)
        return eval_fraction_infix_expression(op, std::make_shared<Ob_Fraction>(std::dynamic_pointer_cast<Ob_Boolean>(left)->m_value, 1), right);

    // string op string
    if (left->type() == Object::OBJECT_STRING && right->type() == Object::OBJECT_STRING)
    {
        auto l = std::dynamic_pointer_cast<Ob_String>(left)->m_value;
        auto r = std::dynamic_pointer_cast<Ob_String>(right)->m_value;
        switch (op)
        {
        case TokenType::PLUS:
            return std::make_shared<Ob_String>(l + r);
        case TokenType::EQUAL_EQUAL:
            return std::make_shared<Ob_Boolean>(l == r);
        case TokenType::BANG_EQUAL:
            return std::make_shared<Ob_Boolean>(l != r);
        default:
            return new_error("Evaluator::eval_infix unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
        }
    }
    // string op int
    if (left->type() == Object::OBJECT_STRING && right->type() == Object::OBJECT_INTEGER)
    {
        auto l = std::dynamic_pointer_cast<Ob_String>(left)->m_value;
        auto r = std::dynamic_pointer_cast<Ob_Integer>(right)->m_value;
        std::string result;
        switch (op)
        {
        case TokenType::STAR:
            for (int i = 0; i < r; i++)
                result += l;
            return std::make_shared<Ob_String>(result);
        case TokenType::DOT:
            if (r < l.length())
                return std::make_shared<Ob_String>(l[r]);
            else
                return new_error("Evaluator::eval_infix: index %d out of length %d", r, l.length());
        default:
            return new_error("Evaluator::eval_infix unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
        }
    }
    if (left->OBJECT_ERROR)
    {
        return left;
    }
    if (right->OBJECT_ERROR)
    {
        return right;
    }

    return new_error("Evaluator::eval_infix unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
}

std::shared_ptr<Object> Evaluator::eval_integer_infix_expression(const TokenType &op, const std::shared_ptr<Object> &left,
                                                                 const std::shared_ptr<Object> &right)
{
    long long l, r;
    if (left->type() == Object::OBJECT_INTEGER)
        l = std::dynamic_pointer_cast<Ob_Integer>(left)->m_value;
    else
        l = std::dynamic_pointer_cast<Ob_Boolean>(left)->m_value;
    if (right->type() == Object::OBJECT_INTEGER)
        r = std::dynamic_pointer_cast<Ob_Integer>(right)->m_value;
    else
        r = std::dynamic_pointer_cast<Ob_Boolean>(right)->m_value;

    switch (op)
    {
    case TokenType::PLUS:
        return std::make_shared<Ob_Integer>(l + r);
    case TokenType::MINUS:
        return std::make_shared<Ob_Integer>(l - r);
    case TokenType::STAR:
        return std::make_shared<Ob_Integer>(l * r);
    case TokenType::SLASH:
        return std::make_shared<Ob_Fraction>(l, r);
    case TokenType::SLASH_SLASH:
        return std::make_shared<Ob_Integer>(l / r);
    case TokenType::PERCENT:
        return std::make_shared<Ob_Integer>(l % r);
    case TokenType::DOT: // 分数
        return std::make_shared<Ob_Fraction>(Ob_Fraction::decimalToFraction(l, r));
    case TokenType::EQUAL_EQUAL:
        return std::make_shared<Ob_Boolean>(l == r);
    case TokenType::BANG_EQUAL:
        return std::make_shared<Ob_Boolean>(l != r);
    case TokenType::LESS:
        return std::make_shared<Ob_Boolean>(l < r);
    case TokenType::GREATER:
        return std::make_shared<Ob_Boolean>(l > r);
    case TokenType::LESS_EQUAL:
        return std::make_shared<Ob_Boolean>(l <= r);
    case TokenType::GREATER_EQUAL:
        return std::make_shared<Ob_Boolean>(l >= r);
    case TokenType::BIT_XOR:
        return std::make_shared<Ob_Integer>(l ^ r);
    case TokenType::BIT_AND:
        return std::make_shared<Ob_Integer>(l & r);
    default:
        return new_error("Evaluator::eval_integer_infix_expression unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
    }
}

std::shared_ptr<Object> Evaluator::eval_fraction_infix_expression(const TokenType &op, const std::shared_ptr<Object> &left,
                                                                  const std::shared_ptr<Object> &right)
{
    auto l = std::dynamic_pointer_cast<Ob_Fraction>(left);
    auto r = std::dynamic_pointer_cast<Ob_Fraction>(right);
    switch (op)
    {
    case TokenType::PLUS:
        return std::make_shared<Ob_Fraction>(Ob_Fraction::add(l, r));
    case TokenType::MINUS:
        return std::make_shared<Ob_Fraction>(Ob_Fraction::sub(l, r));
    case TokenType::STAR:
        return std::make_shared<Ob_Fraction>(Ob_Fraction::mul(l, r));
    case TokenType::SLASH:
        return std::make_shared<Ob_Fraction>(Ob_Fraction::div(l, r));
    case TokenType::PERCENT:
        return std::make_shared<Ob_Fraction>(Ob_Fraction::mod(l, r));
    case TokenType::EQUAL_EQUAL:
        return std::make_shared<Ob_Boolean>(l->equal(r));
    case TokenType::BANG_EQUAL:
        return std::make_shared<Ob_Boolean>(l->notEqual(r));
    case TokenType::LESS:
        return std::make_shared<Ob_Boolean>(l->lessThan(r));
    case TokenType::GREATER:
        return std::make_shared<Ob_Boolean>(l->greaterThan(r));
    case TokenType::LESS_EQUAL:
        return std::make_shared<Ob_Boolean>(l->lessEqual(r));
    case TokenType::GREATER_EQUAL:
        return std::make_shared<Ob_Boolean>(l->greaterEqual(r));
    default:
        return new_error("Evaluator::eval_fraction_infix_expression unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
    }
}
