#pragma once
#include "evaluator.h"

std::shared_ptr<Object> Evaluator::eval_infix(const TokenType op, const std::shared_ptr<Object> &left,
                                              const std::shared_ptr<Object> &right) // 中缀表达式求值
{
    // assign
    if (op == TokenType::EQUAL)
        return eval_assign_expression(left, right);

    // std::cout << left->name().c_str() << TokenTypeToString[op] << right->name().c_str() << std::endl;
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
    default:
        return new_error("Evaluator::eval_fraction_infix_expression unknown operation: %s %s %s", left->name().c_str(), TokenTypeToString[op].c_str(), right->name().c_str());
    }
}

std::shared_ptr<Object> Evaluator::eval_assign_expression(const std::shared_ptr<Object> &name, const std::shared_ptr<Object> &value)
{
    std::cout << "eval_assign_expression: " << name->str() << " = " << value->str() << std::endl;
    return value;
}
