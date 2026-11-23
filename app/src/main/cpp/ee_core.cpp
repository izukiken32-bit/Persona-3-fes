#include "ee_core.hpp"
#include <cstring>
namespace p3fes {
EeCore::EeCore(){reset();}
void EeCore::reset(){s_={};error_="";s_.pc=0xBFC00000;s_.next_pc=s_.pc+4;}
bool EeCore::rd(std::uint32_t a,std::uint32_t&v)const{a&=0x1fffffff;if(a>kRamBytes-4)return false;std::memcpy(&v,ram_.data()+a,4);return true;}
bool EeCore::wr(std::uint32_t a,std::uint32_t v){a&=0x1fffffff;if(a>kRamBytes-4)return false;std::memcpy(ram_.data()+a,&v,4);return true;}
bool EeCore::load_words(std::uint32_t a,const std::uint32_t*w,std::size_t n){if(!w||(a&3)||n>kRamBytes/4){error_="bad load buffer";return false;}for(size_t i=0;i<n;i++)if(!wr(a+std::uint32_t(i*4),w[i])){error_="RAM overflow";return false;}s_.pc=a;s_.next_pc=a+4;return true;}
bool EeCore::step(){std::uint32_t x;if(!rd(s_.pc,x)){error_="fetch fault";return false;}auto old=s_.pc;s_.pc=s_.next_pc;s_.next_pc+=4;unsigned op=x>>26,rs=(x>>21)&31,rt=(x>>16)&31,rdx=(x>>11)&31,fn=x&63,sh=(x>>6)&31;auto imm=std::int16_t(x);
switch(op){case 0:switch(fn){case 0:s_.gpr[rdx]=s_.gpr[rt]<<sh;break;case 2:s_.gpr[rdx]=s_.gpr[rt]>>sh;break;case 8:s_.next_pc=std::uint32_t(s_.gpr[rs]);break;case 0x21:s_.gpr[rdx]=s_.gpr[rs]+s_.gpr[rt];break;case 0x23:s_.gpr[rdx]=s_.gpr[rs]-s_.gpr[rt];break;case 0x24:s_.gpr[rdx]=s_.gpr[rs]&s_.gpr[rt];break;case 0x25:s_.gpr[rdx]=s_.gpr[rs]|s_.gpr[rt];break;default:error_="unsupported SPECIAL";return false;}break;
case 9:s_.gpr[rt]=s_.gpr[rs]+imm;break;case 12:s_.gpr[rt]=s_.gpr[rs]&(x&65535);break;case 13:s_.gpr[rt]=s_.gpr[rs]|(x&65535);break;case 15:s_.gpr[rt]=std::uint64_t(x&65535)<<16;break;
case 35:{std::uint32_t v;if(!rd(std::uint32_t(s_.gpr[rs]+imm),v)){error_="LW fault";return false;}s_.gpr[rt]=std::int32_t(v);break;}
case 43:if(!wr(std::uint32_t(s_.gpr[rs]+imm),std::uint32_t(s_.gpr[rt]))){error_="SW fault";return false;}break;
case 4:if(s_.gpr[rs]==s_.gpr[rt])s_.next_pc=old+4+(std::int32_t(imm)<<2);break;
case 5:if(s_.gpr[rs]!=s_.gpr[rt])s_.next_pc=old+4+(std::int32_t(imm)<<2);break;
default:error_="unsupported opcode";return false;}s_.gpr[0]=0;++s_.cycles;return true;}
}