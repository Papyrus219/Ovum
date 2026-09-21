#ifndef OVUM_FORMULA_HPP
#define OVUM_FORMULA_HPP

#include <Ovum/simulation_scene.hpp>

namespace ovum
{

using Getter = float (*)(const Entiety_data &);
using Registry = std::unordered_map<std::string, Getter>;

inline Registry Make_registry()
{
    return {
        {"speed", [](const Entiety_data & data) {return data.speed;}},
        {"size", [](const Entiety_data & data) {return data.size;}},
        {"sense", [](const Entiety_data & data) {return data.sense;}},
        {"energy", [](const Entiety_data & data) {return data.energy;} },
        {"food_eaten", [](const Entiety_data & data) {return static_cast<float>(data.food_eaten);}}
    };
}

class Formula
{
public:
    static Formula Compile(std::string_view text, const Registry & reg);

    float Evaluate(const Entiety_data & data) const;
private:
    static constexpr int MAX_STACK{32};
    enum class Op { CONST, VAR, ADD, SUB, MUL, DIV, NEG};
    struct Instruction
    {
        Op op{};
        float value{};
        int variable{};
    };

    std::vector<Instruction> code_{};
    std::vector<Getter> varables_{};
    int depth_{};

    void Emit(Instruction instruction);

    struct Parser
    {
        std::string_view formula_string{};
        const Registry & reg;
        Formula & formula;
        size_t pos{};

        void Skip_white_spaces();

        bool Accept(char c);

        void Parse_expresion();
        void Parse_term();
        void Parse_factor();
    };
};

}

#endif // OVUM_FORMULA_HPP
