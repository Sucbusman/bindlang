#include <bits/stdint-uintn.h>
#include <iostream>
#include "vm/value.h"
#include "vm/vm.h"
#include "vm/coder.h"

namespace bindlang { namespace vm{

using std::cout;
using std::cerr;
using std::endl;

template <class... Args>
bool VM::error(Args...args){
  (cerr<< ... <<args)<<endl;
  return false;
}

void VM::reset(){
  ip = (this->rom).data();
  syscalls.clear();
  standardSyscalls();
  frames.clear();
  frames.push_back(callFrame(0,0,0,nullptr));
  bp  = 0;
  sp  = 0;
  vsp = nullptr;
}

void VM::init(vector<std::uint8_t> &&codes,
              vector<Value> &&constants){
  this->rom = codes;
  this->constants=constants;
  values.clear();
  reset();
}

void VM::standardSyscalls(){
  syscalls.push_back(sys_print);
  syscalls.push_back(sys_inspect);
}

void VM::push(Value val){
  if(sp<values.size()){
    values[sp] = val;
    sp++;
  }else{
    values.push_back(val);
    sp = values.size();
  }
}

Value VM::pop(){
  sp--;
  return values[sp];
}

void printIndent(int n){
  while(n--)
    cout<<' ';
}

void VM::dumpRegs(){
  cout<<"  frame:"<<frames.size()
      <<" sp:0x"<<hex<<sp<<" bp:0x"<<hex<<bp<<" val_reg:";
  printVal(reg_val);
  cout<<endl;
}

void VM::dumpStack(){
  for(int it=sp-1;it>=0;it--){
    if(it==bp){
      printIndent(3);
      cout<<'>';
    }
    else {
      printIndent(4);
    }
    inspectVal(values[it]);
    cout<<endl;
  }
  cout<<"    ------------------------------------"<<endl;
}
bool VM::run(){
#define EAT(T) (*(T*)ip);ip+=sizeof(T)
#define PEEK(T) (*(T*)ip)
#define bytep  ((uint8_t*)ip)
#define wordp  ((uint32_t*)ip)
#define dwordp ((uint64_t*)ip)
#define WHEN(OP) case (uint8_t)OpCode::OP
#define BINARY_OP(type,op,rst_type)            \
  do{auto r = pop();auto l = pop();            \
    rst_type ans = (l.as.type op r.as.type);   \
  reg_val = Value(ans);}while(0)
#define EXPECT(T) \
  if(reg_val.type != T){error("Expect " #T);break;}
  uint32_t word;
  uint64_t dword;
  while(true){
    if(debug){
      disas_inst(ip);//debug
    }
    uint8_t opc = EAT(uint8_t);
    switch(opc){
      WHEN(SETG):{
        word = EAT(uint32_t);
        values[word]=reg_val;
        break;
      }
      WHEN(GETG):{
        word = EAT(uint32_t);
        reg_val = values[word];
        break;
      }
      WHEN(SETC):{
        word = EAT(uint32_t);
        if(vsp){
          (*vsp)[word] = reg_val;
        }else{
          error("captured values ptr is null");
        }
        break;
      }
      WHEN(GETC):{
        word = EAT(uint32_t);
        if(vsp){
          reg_val = (*vsp)[word];
        }else{
          error("captured values ptr is null");
        }
        break;
      }
      WHEN(SETL):{
        word = EAT(uint32_t);
        values[bp+word] = reg_val;
        break;
      }
      WHEN(GETL):{
        word = EAT(uint32_t);
        reg_val = values[bp+word];
        break;
      }
      WHEN(SYSCALL):
        word = EAT(uint32_t);
        if(not syscalls[word](*this)){
          return false;
        }
        break;
      WHEN(CAPTURE):{
        auto val_info = EAT(uint32_t);
        while(val_info != 0u){
          bool localp = val_info & (1u<<30);
          auto idx = val_info & ~(3u<<30);
          auto proc = AS_PROCEDURE(reg_val);
          if(localp){
            proc->captureds.push_back(values[bp+idx]);
          }else{
            proc->captureds.push_back((*vsp)[idx]);
          }
          val_info = EAT(uint32_t);
        }
        break;
      }
      WHEN(CNST):{
        word = EAT(uint32_t);
        reg_val = constants[word];
        break;
      }
      WHEN(CNSH):
        reg_val = constants[*dwordp];
        ip+=8;
        break;
      WHEN(IMM):
        dword = EAT(uint64_t);
        reg_val = Value(dword);
        break;
      WHEN(ADD):    BINARY_OP(number,+,uint64_t);break;
      WHEN(MINUS):  BINARY_OP(number,-,uint64_t);break;
      WHEN(MULT):   BINARY_OP(number,*,uint64_t);break;
      WHEN(DIVIDE): BINARY_OP(number,/,uint64_t);break;
      WHEN(GT):     BINARY_OP(number,>,bool);break;
      WHEN(LT):     BINARY_OP(number,<,bool);break;
      WHEN(TRUE):reg_val = Value(true);break;
      WHEN(FALSE):reg_val = Value(false);break;
      WHEN(EQ):{
        auto r = pop();
        auto l = pop();
        reg_val = Value(valueEqual(r, l));
        break;
      }
      WHEN(UNIT):
        reg_val = Value(make_obj(Value(false),nullptr));
        break;
      WHEN(RCONS):{
        auto l = pop();
        auto r = pop();
        reg_val = Value(make_obj(l,AS_LIST(r)));
        break;
      }
      WHEN(CONS):{
        auto r = pop();
        auto l = pop();
        reg_val = Value(make_obj(l,AS_LIST(r)));
        break;
      }
      WHEN(HEAD):{
        reg_val = pop();
        EXPECT(VAL_List);
        auto lst = AS_LIST(reg_val);
        reg_val = lst->head;
        break;
      }
      WHEN(TAIL):{
        reg_val = pop();
        EXPECT(VAL_List);
        auto lst = AS_LIST(reg_val);
        reg_val = Value(lst->tail);
        break;
      }
      WHEN(EMPTYP):{
        reg_val = pop();
        EXPECT(VAL_List);
        auto lst = AS_LIST(reg_val);
        if(lst->tail){
          reg_val = Value(false);
        }else{
          reg_val = Value(true);
        }
        break;
      }
      WHEN(PUSH):
        push(reg_val);
        break;
      WHEN(POP):
        reg_val = pop();
        break;
      WHEN(TCALL):
      WHEN(CALL):{
         auto func = AS_PROCEDURE(reg_val);
         frames.push_back(callFrame(ip,bp,sp-func->arity,vsp));
         vsp = &func->captureds;
         bp = sp-func->arity;
         ip = rom.data()+func->offset;//jump
         break;
      }
      WHEN(JMP):
        ip += *(int32_t*)ip-1;break;
      WHEN(JNE):{
        if(not truthy(reg_val)){
          ip += *(int32_t*)ip-1;
        }else {
          EAT(uint32_t);
        }
        break;
      }
      WHEN(RET):{
        sp = frames.back().sp;
        bp = frames.back().bp;
        ip = frames.back().ip;
        vsp = frames.back().vsp;
        frames.pop_back();
        break;
      }
      WHEN(HALT):{
        return true;
      }
      default:{
        cerr<<opcTable[opc]<<" not implemented in vm run!"<<endl;
        return false;
      }
    }
    if(debug){
      dumpRegs();//debug
      dumpStack();
    }
  }
  return false;
#undef BINARY_OP
#undef PEEK
#undef EXPECT
#undef EAT
}

uint8_t* VM::disas_inst(uint8_t* pc){
#define EAT(T) (*(T*)pc);pc+=sizeof(T)
  uint8_t opc = EAT(uint8_t);
  cout<<"0x"<<left<<setw(4)<<hex<<pc-1-rom.data()
      <<GREENCODE<<opcTable[opc]<<DEFAULT<<' ';
  switch (opc){
    WHEN(SETL):
    WHEN(GETL):
    WHEN(SETC):
    WHEN(GETC):
    WHEN(SETG):
    WHEN(GETG):{
      int32_t n = EAT(int32_t);
      cout<<dec<<n<<endl;
      break;
    }
    WHEN(CNST):{
      uint32_t n = EAT(uint32_t);
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
    WHEN(JMP):{
      int32_t n = EAT(int32_t);
      auto jmp_ip = pc-5-rom.data()+n;
      cout<<dec<<n<<"  #0x"<<hex<<jmp_ip<<endl;
      break;
    }
    WHEN(SYSCALL):{
      uint32_t n = EAT(uint32_t);
      cout<<dec<<n<<endl;
      break;
    }
    WHEN(CAPTURE):{
      auto val_info = EAT(uint32_t);
      cout<<"#";
      while(val_info != 0u){
        bool localp = val_info & (1u<<30);
        auto idx = val_info & ~(3u<<30);
        cout<<"<"<<localp<<":"<<idx<<"> ";
        val_info = EAT(uint32_t);
      }
      cout<<endl;
      break;
    }
    WHEN(TRUE):
    WHEN(FALSE):
    WHEN(ADD):
    WHEN(MINUS):
    WHEN(MULT):
    WHEN(DIVIDE):
    WHEN(GT):
    WHEN(LT):
    WHEN(EQ):
    WHEN(UNIT):
    WHEN(CONS):
    WHEN(RCONS):
    WHEN(HEAD):
    WHEN(TAIL):
    WHEN(EMPTYP):
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

void VM::dumpConstant(){
  for(auto & v:constants){
    inspectVal(v);
    cout<<endl;
  }
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

bool sys_print(VM& vm){
  printVal(vm.reg_val);
  return true;
}

bool sys_inspect(VM& vm){
  inspectVal(vm.reg_val);
  return true;
}

} }
