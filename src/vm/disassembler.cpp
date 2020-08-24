#include "vm.h"
namespace bindlang::vm {

void VM::dumpConstant(){
  for(auto & v:constants){
    inspectVal(v);
    cout<<endl;
  }
}

uint8_t* VM::disas_inst(uint8_t* pc){
#define WHEN(OP) case (uint8_t)OpCode::OP
#define EAT(T) (*(T*)pc);pc+=sizeof(T)
  uint8_t opc = EAT(uint8_t);
  cout<<"0x"<<left<<setw(4)<<hex<<pc-1-rom.data()
      <<GREENCODE<<opcTable[opc]<<DEFAULT<<' ';
  switch (opc){
    WHEN(SETL):
    WHEN(GETL):{
      auto n = EAT(int16_t);
      cout<<dec<<n<<endl;
      break;
    }
    WHEN(SETC):
    WHEN(GETC):{
      auto n = EAT(int8_t);
      cout<<dec<<(int)n<<endl;
      break;
    }
    WHEN(CNST):{
      auto n = EAT(uint32_t);
      cout<<dec<<n<<"  #";
      inspectVal(constants[n]);
      cout<<endl;
      break;
    }
    WHEN(CNSH):{
      uint64_t n = EAT(uint64_t);
      cout<<dec<<n<<"  #";
      inspectVal(constants[n]);
      cout<<endl;
      break;
    }
    WHEN(IMM):{
      uint64_t n = EAT(uint64_t);
      cout<<dec<<n<<endl;
      break;
    }
    WHEN(JNE):
    WHEN(JEQ):
    WHEN(JMP):{
      int16_t n = EAT(int16_t);
      auto jmp_ip = pc-1-sizeof(decltype(n))-rom.data()+n;
      cout<<dec<<n<<"  #0x"<<hex<<jmp_ip<<endl;
      break;
    }
    WHEN(SYSCALL):{
      auto n = EAT(uint8_t);
      cout<<dec<<n<<endl;
      break;
    }
    WHEN(CAPTURE):{
      auto val_info = EAT(uint8_t);
      cout<<"#";
      while(val_info != 0u){
        bool localp = val_info & (1u<<6);
        auto idx = val_info & ~(3u<<6);
        cout<<"<"<<dec<<localp<<":"<<dec<<idx<<"> ";
        val_info = EAT(uint8_t);
      }
      cout<<endl;
      break;
    }
    WHEN(START):
    WHEN(TRUE):
    WHEN(FALSE):
    WHEN(NOT):
    WHEN(ADD):
    WHEN(MINUS):
    WHEN(MULT):
    WHEN(DIVIDE):
    WHEN(GT):
    WHEN(LT):
    WHEN(EQ):
    WHEN(COPY):
    WHEN(UNIT):
    WHEN(CONS):
    WHEN(RCONS):
    WHEN(HEAD):
    WHEN(TAIL):
    WHEN(EMPTYP):
    WHEN(NOP):
    WHEN(PUSH):
    WHEN(POP):
    WHEN(CALL):
    WHEN(TCALL):
    WHEN(HALT):
    WHEN(RET):
      cout<<endl;
      break;
    default:{
      cout<<opcTable[opc]<<" disassemble unimplemented!"<<endl;
      break;
    }
  }
  return pc;
#undef EAT
}

void VM::disassemble(){
  auto pc = ip;
  dumpConstant();
  cout<<"=============="<<endl;
  while(pc-rom.data()<rom.size()){
    pc=disas_inst(pc);
  }
#undef WHEN
}

}
