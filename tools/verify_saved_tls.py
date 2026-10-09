"""Bounded x86 finish-extent check, using objdump instructions/relocations.

Only the supported scalar entry/cleanup shapes are admitted. This is not a
general x86 executor and does not execute native code or prove runtime lifetime.
"""
import re

UNKNOWN=('unknown',)
SAVED=('saved_tls_byte',)
TEB=('teb_tls_array',)
INDEX=('tls_index',)

def require(value,message):
    if not value:raise ValueError('Saved TLS proof: '+message)

def nodes(body):
    result=[]
    for line in body.splitlines():
        if re.search(r'\b(?:dir32|secrel32|DISP32)\s',line):
            require(result,'relocation without instruction');result[-1][2].append(line.strip());continue
        columns=line.split('\t')
        if len(columns)>=3 and re.fullmatch(r'\s*[0-9a-f]+:',columns[0]):
            op=' '.join(columns[-1].split()).split(' <')[0]
            if re.match('[a-z]',op):
                result.append((int(columns[0].strip()[:-1],16),op,[]))
    require(result,'missing instructions')
    return result

def pointer(zone,offset):return ('pointer',zone,offset)

class State:
    def __init__(self,regs=None,memory=None,restored=False,entry=False):
        self.regs=dict(regs or {});self.memory=dict(memory or {});self.restored=restored;self.entry=entry
    def clone(self):return State(self.regs,self.memory,self.restored,self.entry)
    def register(self,name):return self.regs.get(name,UNKNOWN)
    def set_register(self,name,value):
        if re.fullmatch('e[abcd]x',name):
            self.regs[name]=value
            self.regs[name[1]+'l']=value if value==SAVED else ('constant',value[1]&255) if value[:1]==('constant',) else UNKNOWN
        elif re.fullmatch('[abcd]l',name):
            self.regs['e'+name[0]+'x']=UNKNOWN;self.regs[name]=value
        else:self.regs[name]=value
    def address(self,operand):
        if operand=='fs:0x2c':return TEB
        match=re.fullmatch(r'\[([a-z]{3})(?:\+([a-z]{3})\*4)?(?:([+-])0x([a-f0-9]+))?\]',operand)
        if not match:return UNKNOWN
        base=self.register(match[1])
        if match[2]:return ('tls_slot',) if base==TEB and self.register(match[2])==INDEX and not match[3] else UNKNOWN
        if base[:1]!=('pointer',):return UNKNOWN
        displacement=int(match[4],16) if match[4] else 0
        if match[3]=='-':displacement=-displacement
        return pointer(base[1],base[2]+displacement)
    def read(self,operand,relocations,state_offset,implicit_width=4):
        if re.fullmatch(r'[a-z]{2,3}',operand):return self.register(operand)
        if re.fullmatch(r'0x[a-f0-9]+',operand):return ('constant',int(operand,16))
        if operand=='fs:0x2c':return TEB if implicit_width==4 else UNKNOWN
        if operand.startswith('ds:'):return INDEX if implicit_width==4 and any('dir32\t_tls_index' in r for r in relocations) else UNKNOWN
        match=re.fullmatch(r'(BYTE|DWORD) PTR (.+)',operand)
        if not match:return UNKNOWN
        width=1 if match[1]=='BYTE' else 4
        if match[2]=='fs:0x2c':return TEB if width==4 else UNKNOWN
        if match[2].startswith('ds:'):
            return INDEX if width==4 and any('dir32\t_tls_index' in r for r in relocations) else UNKNOWN
        address=self.address(match[2])
        if address==('tls_slot',) and width==4:return pointer('tls',0)
        if address==pointer('tls',state_offset) and width==1 and any('secrel32\t.tls$' in r for r in relocations):return SAVED if self.entry else UNKNOWN
        return self.memory.get((address,width),UNKNOWN)
    def write(self,operand,value,relocations,state_offset):
        if re.fullmatch(r'(?:e[abcd]x|e[sd]i|eb[px]|esp|[abcd]l)',operand):
            self.set_register(operand,value);return
        match=re.fullmatch(r'(BYTE|DWORD) PTR (.+)',operand)
        require(match,'unsupported destination '+operand)
        width=1 if match[1]=='BYTE' else 4;address=self.address(match[2])
        require(address!=UNKNOWN or (match[2].startswith('ds:') and any('dir32\t.bss' in r for r in relocations)),
                'unproved store address')
        if address[:2]==('pointer','tls') and address[2]<=state_offset<address[2]+width:
            require(width==1 and any('secrel32\t.tls$' in r for r in relocations),'unidentified TLS destination')
            require(address[2]==state_offset and value==SAVED,'TLS store is not the saved entry byte');self.restored=True
        if address!=UNKNOWN:
            if address[:1]==('pointer',):
                for key in list(self.memory):
                    previous,span=key
                    if previous[:1]==('pointer',) and previous[1]==address[1] and previous[2]<address[2]+width and address[2]<previous[2]+span:
                        del self.memory[key]
            self.memory[(address,width)]=value
    def step(self,op,relocations,state_offset):
        mnemonic,*rest=op.split(' ',1);args=rest[0].split(',') if rest else []
        if mnemonic in ('mov','movzx'):
            require(len(args)==2,'invalid move')
            self.write(args[0],self.read(args[1],relocations,state_offset,1 if re.fullmatch('[abcd]l',args[0]) else 4),relocations,state_offset)
        elif mnemonic=='lea':
            require(len(args)==2,'invalid lea');self.set_register(args[0],self.address(args[1]))
        elif mnemonic=='push':
            value=self.read(args[0],relocations,state_offset);esp=self.register('esp')
            require(esp[:1]==('pointer',),'unknown stack');esp=pointer(esp[1],esp[2]-4)
            self.set_register('esp',esp);self.memory[(esp,4)]=value
        elif mnemonic=='pop':
            esp=self.register('esp');require(esp[:1]==('pointer',),'unknown stack')
            self.set_register(args[0],self.memory.get((esp,4),UNKNOWN));self.set_register('esp',pointer(esp[1],esp[2]+4))
        elif mnemonic=='leave':
            self.set_register('esp',self.register('ebp'));self.step('pop ebp',[],state_offset)
        elif mnemonic in ('add','sub','and','or','xor','shl','sete','setne'):
            require(args,'missing scalar destination')
            value=UNKNOWN
            if mnemonic in ('add','sub') and len(args)==2 and re.fullmatch('0x[a-f0-9]+',args[1]):
                old=self.register(args[0])
                if old[:1]==('pointer',):value=pointer(old[1],old[2]+int(args[1],16)*(1 if mnemonic=='add' else -1))
            self.write(args[0],value,relocations,state_offset)
        elif mnemonic in ('test','cmp','nop','xchg'):
            require(mnemonic!='xchg' or args==['ax','ax'],'unsupported exchange')
        elif mnemonic=='lock':
            # Owner retirement is disjoint global storage, not a TLS/stack
            # store. CMPXCHG can replace EAX on failure; forget its prior value.
            # This saved-byte proof does not certify the owner's CAS semantics.
            require(re.fullmatch(r'cmpxchg DWORD PTR ds:0x[a-f0-9]+,e(?:ax|bx|cx|dx|si|di|bp)',rest[0]) and
                    len(relocations)==1 and 'dir32\t.bss' in relocations[0],
                    'unproved compare-exchange destination')
            self.set_register('eax',UNKNOWN)
        else:raise ValueError('Saved TLS proof: unsupported instruction '+op)

def verify_saved_tls(entry_body,finish_body,state_offset):
    entry=nodes(entry_body);initial=State({'esp':pointer('entry_stack',0)},entry=True)
    context=None
    for _,op,relocations in entry:
        if op.startswith('call '):
            require(any('DISP32\tss2vrNativeFinally' in r for r in relocations),'unexpected entry call')
            stack=initial.register('esp');context=initial.memory.get((pointer(stack[1],stack[2]+8),4),UNKNOWN);break
        initial.step(op,relocations,state_offset)
    require(context and context[:1]==('pointer',),'unproved finally context')
    require(SAVED in initial.memory.values(),'entry did not save the symbol-resolved native TLS byte')
    # Preserve references and the immutable saved byte; mutable run-body values
    # cannot be inferred from entry initialization (notably entered trace=null).
    memory={key:value for key,value in initial.memory.items() if value==SAVED or value[:1]==('pointer',)}
    closure=memory.get((pointer(context[1],context[2]+4),4),UNKNOWN)
    require(closure[:1]==('pointer',),'unproved cleanup capture')
    # The source-reviewed cleanup captures only immutable wasActive by reference,
    # and optionally entered (an invocation-local typed trace borrow). Its pointee
    # is disjoint from this entry frame. This proof does not establish its lifetime.
    borrowed=0
    for offset in (0,4):
        reference=memory.get((pointer(closure[1],closure[2]+offset),4),UNKNOWN)
        if reference[:1]!=('pointer',):continue
        if memory.get((reference,1))==SAVED:continue
        if (reference,4) in initial.memory:
            memory[(reference,4)]=pointer('borrowed_trace',0);borrowed+=1
    require(borrowed<=1,'unsupported mutable capture layout')
    memory[(pointer('finish_stack',4),4)]=context
    start=State({'esp':pointer('finish_stack',0)},memory)
    finish=nodes(finish_body);locations={address:i for i,(address,_,_) in enumerate(finish)}
    work=[(0,start)];visited=set();exits=0
    while work:
        index,state=work.pop()
        require(0<=index<len(finish),'unhandled fallthrough')
        key=(index,tuple(sorted(state.regs.items())),tuple(sorted(state.memory.items())),state.restored)
        if key in visited:continue
        visited.add(key);require(len(visited)<=4096,'cleanup state budget exceeded')
        _,op,relocations=finish[index];mnemonic=op.split(' ',1)[0]
        if mnemonic=='ret' or (mnemonic=='jmp' and any('DISP32\tss2vr::game::nativeUiFault' in r for r in relocations)):
            require(state.restored,'exit bypasses saved TLS restoration');exits+=1;continue
        if mnemonic.startswith('j'):
            target=re.fullmatch(r'j[a-z]+ ([a-f0-9]+)',op)
            require(target and int(target[1],16) in locations,'unhandled cleanup edge')
            work.append((locations[int(target[1],16)],state.clone()))
            if mnemonic!='jmp':work.append((index+1,state))
        else:
            state.step(op,relocations,state_offset);work.append((index+1,state))
    require(exits>=2,'missing normal/abnormal exit coverage')
    return {'reachable_exits':exits,'states_checked':len(visited),'saved_origin':'entry native TLS via immutable capture'}
