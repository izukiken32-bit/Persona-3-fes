#pragma once
#include <array>
#include <cstdint>
namespace p3fes {
struct EeState { std::array<std::uint64_t,32> gpr{}; std::uint64_t hi{},lo{},cycles{}; std::uint32_t pc{},next_pc{}; };
class EeCore {
public: static constexpr std::size_t kRamBytes=32u*1024u*1024u;
 EeCore(); void reset(); bool load_words(std::uint32_t,const std::uint32_t*,std::size_t); bool step();
 const EeState& state() const{return s_;} const char* error()const{return error_;}
private: bool rd(std::uint32_t,std::uint32_t&)const; bool wr(std::uint32_t,std::uint32_t);
 EeState s_{}; std::array<std::uint8_t,kRamBytes> ram_{}; const char* error_="";
};}