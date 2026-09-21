#include "Ovum/formula.hpp"

using namespace ovum;

Formula ovum::Formula::Compile(std::string_view text, const Registry & reg)
{
    Formula formula{};
    Parser parser{text, reg, formula};

    parser.Parse_expresion();
    parser.Skip_white_spaces();
    if(parser.pos != text.size())
    {
        throw std::runtime_error{"Unexpected character in formula: " + std::string{text}};
    }

    return formula;
}

float ovum::Formula::Evaluate(const Entiety_data & data) const
{
    float stack[MAX_STACK]{};
    int stack_pointer{};

    for(const Instruction & instr : code_)
    {
        switch(instr.op)
        {
            case Op::CONST:
                stack[stack_pointer++] = instr.value;
                break;
            case Op::VAR:
                stack[stack_pointer++] = varables_[instr.variable](data);
                break;
            case Op::NEG:
                stack[stack_pointer - 1] = -stack[stack_pointer - 1];
                break;
            case Op::ADD:
                stack_pointer--;
                stack[stack_pointer - 1] += stack[stack_pointer];
                break;
            case Op::SUB:
                stack_pointer--;
                stack[stack_pointer - 1] -= stack[stack_pointer];
                break;
            case Op::MUL:
                stack_pointer--;
                stack[stack_pointer - 1] *= stack[stack_pointer];
                break;
            case Op::DIV:
                stack_pointer--;
                stack[stack_pointer - 1] /= stack[stack_pointer];
                break;
        }
    }

    return stack[0];
}

void ovum::Formula::Emit(Instruction instruction)
{
    switch(instruction.op)
    {
        case Op::CONST:
        case Op::VAR:
            depth_++;
            break;
        case Op::NEG:
            break;
        default:
            depth_--;
            break;
    }

    if(depth_ >= MAX_STACK)
    {
        throw std::runtime_error{"Formula too complex"};
    }

    code_.push_back(instruction);
}

void ovum::Formula::Parser::Skip_white_spaces()
{
    while(pos < formula_string.size() && std::isspace(static_cast<unsigned char>(formula_string[pos])))
    {
        pos++;
    }
}

bool ovum::Formula::Parser::Accept(char c)
{
    Skip_white_spaces();
    if(pos < formula_string.size() && formula_string[pos] == c)
    {
        pos++;
        return true;
    }

    return false;
}

void ovum::Formula::Parser::Parse_expresion()
{
    Parse_term();
    for(;;)
    {
        if(Accept('+'))
        {
            Parse_term();
            formula.Emit({Op::ADD});
        }
        else if(Accept('-'))
        {
            Parse_term();
            formula.Emit({Op::SUB});
        }
        else
        {
            return;
        }
    }
}

void ovum::Formula::Parser::Parse_term()
{
    Parse_factor();
    for(;;)
    {
        if(Accept('*'))
        {
            Parse_factor();
            formula.Emit({Op::MUL});
        }
        else if(Accept('/'))
        {
            Parse_factor();
            formula.Emit({Op::DIV});
        }
        else
        {
            return;
        }
    }
}

void ovum::Formula::Parser::Parse_factor()
{
    Skip_white_spaces();
    if( Accept('-') )
    {
        Parse_factor();
        formula.Emit({Op::NEG});
        return;
    }
    if( Accept('(') )
    {
        Parse_expresion();
        if( !Accept(')') )
        {
            throw std::runtime_error{"Missing ')'"};
        }
        return;
    }
    if(pos < formula_string.size() && (std::isdigit(static_cast<unsigned char>(formula_string[pos]) || formula_string[pos] == '.')))
    {
        size_t used{};
        float value = std::stof(std::string{formula_string.substr(pos)}, &used);
        pos += used;
        formula.Emit({Op::CONST, value});
        return;
    }
    if(pos < formula_string.size() && (std::isalpha(static_cast<unsigned char>(formula_string[pos]) || formula_string[pos] == '_')))
    {
        size_t start = pos;
        while(pos < formula_string.size() && (std::isalnum(static_cast<unsigned char>(formula_string[pos]) || formula_string[pos] == '_')))
        {
            pos++;
        }

        std::string name{formula_string.substr(start, pos - start)};
        auto it = reg.find(name);
        if(it == reg.end())
        {
            throw std::runtime_error{"Unknown variable: " + name};
        }

        formula.varables_.push_back(it->second);
        formula.Emit({Op::VAR, 0.0f, static_cast<int>(formula.varables_.size() - 1)});
        return;
    }

    throw std::runtime_error{"Unexpected token in formula"};
}







