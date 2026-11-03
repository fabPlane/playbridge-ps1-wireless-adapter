const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const html = fs.readFileSync(path.join(__dirname, '../controller_page.h'), 'utf8');
const js = html.match(/<script>([\s\S]*?)<\/script>/)[1];
const bits = [8,10,11,9,4,7,5,6,0,3,12,13,14,15];
function element(bit) {
  return {dataset:{bit:String(bit)}, events:{}, textContent:'', held:false,
    addEventListener(name, cb){this.events[name]=cb;},setPointerCapture(){},setAttribute(){},
    classList:{toggle(name,value){this.value=value;}}};
}
const buttons=bits.map(element), result=element(), diagnostics=element(), neutral=element();
const docEvents={}, winEvents={}, intervals=[], sent=[], beacons=[];
let fail=false;
const context={
  document:{hidden:false,getElementById(id){return {result,diagnostics,neutral}[id];},
    querySelectorAll(){return buttons;},addEventListener(n,f){docEvents[n]=f;}},
  window:{addEventListener(n,f){winEvents[n]=f;}},
  navigator:{sendBeacon(url,body){beacons.push([url,body.toString()]);}},
  URLSearchParams,AbortSignal,
  setInterval(fn){intervals.push(fn);},setTimeout(){},
  async fetch(url,options={}){
    if(url==='/status')return {ok:true,json:async()=>({fullReplies:12,dataMismatches:0,rx:'01 42',data:'FF 41'})};
    sent.push(Number(new URLSearchParams(options.body).get('mask')));
    if(fail)throw Error('offline');
    return {ok:true,text:async()=> 'Received'};
  }
};
vm.createContext(context);vm.runInContext(js,context);
const flush=()=>new Promise(resolve=>setImmediate(resolve));
const button=bit=>buttons.find(b=>Number(b.dataset.bit)===bit);
const event=id=>({pointerId:id,preventDefault(){}});
const key=(code,other={})=>({code,target:{tagName:'BODY'},preventDefault(){},...other});
(async()=>{
  await flush();assert.equal(sent.at(-1),65535);
  button(4).events.pointerdown(event(1));await flush();assert.equal(sent.at(-1),0xFFEF);
  intervals[0]();await flush();assert.equal(sent.at(-1),0xFFEF);
  button(14).events.pointerdown(event(2));await flush();assert.equal(sent.at(-1),0xBFEF);
  button(4).events.pointerup(event(1));await flush();assert.equal(sent.at(-1),0xBFFF);
  button(14).events.pointercancel(event(2));await flush();assert.equal(sent.at(-1),0xFFFF);
  // Immediate press/release remains serialized; release is the final state.
  button(6).events.pointerdown(event(3));button(6).events.pointerup(event(3));
  await flush();assert.equal(sent.at(-1),0xFFFF);
  button(4).events.pointerdown(event(4));await flush();winEvents.blur();await flush();assert.equal(sent.at(-1),0xFFFF);
  button(5).events.pointerdown(event(5));await flush();neutral.events.click();await flush();assert.equal(sent.at(-1),0xFFFF);
  fail=true;button(4).events.pointerdown(event(6));await flush();
  assert.match(result.textContent,/Connection lost/);
  const count=sent.length;intervals[0]();await flush();assert.equal(sent.length,count);
  fail=false;winEvents.pagehide();assert.deepEqual(beacons.at(-1),['/state','mask=65535']);
  // Every advertised mapping uses the correct active-low PS1 bit.
  const mappings={ArrowUp:4,KeyW:4,ArrowRight:5,KeyD:5,ArrowDown:6,KeyS:6,ArrowLeft:7,KeyA:7,KeyI:12,KeyL:13,KeyK:14,KeyJ:15,KeyQ:10,KeyE:11,Digit1:8,Digit3:9,ShiftLeft:0,ShiftRight:0,Enter:3};
  for(const [code,bit] of Object.entries(mappings)){
    winEvents.keydown(key(code));await flush();assert.equal(sent.at(-1),65535&~(1<<bit),code);
    winEvents.keyup(key(code));await flush();assert.equal(sent.at(-1),65535,code+' release');
  }
  winEvents.keydown(key('KeyW'));winEvents.keydown(key('ArrowUp'));winEvents.keydown(key('KeyK'));
  await flush();assert.equal(sent.at(-1),0xBFEF);
  winEvents.keyup(key('KeyW'));await flush();assert.equal(sent.at(-1),0xBFEF,'alias remains down');
  button(4).events.pointerdown(event(7));winEvents.keyup(key('ArrowUp'));await flush();assert.equal(sent.at(-1),0xBFEF,'touch remains down');
  button(4).events.pointerup(event(7));await flush();assert.equal(sent.at(-1),0xBFFF);
  const before=sent.length;winEvents.keydown(key('KeyK',{repeat:true}));await flush();assert.equal(sent.length,before);
  winEvents.keydown(key('Escape'));await flush();assert.equal(sent.at(-1),65535);
  for(const overrides of [{ctrlKey:true},{metaKey:true},{altKey:true},{target:{tagName:'INPUT'}},{target:{isContentEditable:true}}]){
    winEvents.keydown(key('KeyW',overrides));await flush();assert.equal(sent.at(-1),65535);
  }
  winEvents.keydown(key('KeyJ'));await flush();context.document.hidden=true;docEvents.visibilitychange();await flush();assert.equal(sent.at(-1),65535);
  winEvents.keydown(key('KeyL'));await flush();winEvents.blur();await flush();assert.equal(sent.at(-1),65535);
  console.log('PASS: pointer/multitouch, all 20 keyboard mappings, mixed chords, aliases, repeats, shortcuts/editable fields, visibility, blur, neutral, network failure and pagehide.');
})().catch(e=>{console.error(e);process.exitCode=1;});
