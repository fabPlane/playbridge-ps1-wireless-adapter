#pragma once
static const char CONTROLLER_PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PlayBridge Controller</title><style>
body{font:18px system-ui;max-width:38rem;margin:2rem auto;padding:1rem}
button{font:inherit;padding:.8rem;margin:.25rem;touch-action:none;user-select:none}
.held{background:#1769e0;color:white}.dpad{display:grid;grid-template-columns:repeat(3, max-content)}
section{margin:1rem 0}#diagnostics{font-size:14px;color:#444;white-space:pre-wrap}
</style></head><body><h1>PlayBridge Controller</h1>
<p>Hold to press; release to stop. Disconnecting releases buttons automatically.</p>
<section><button data-bit="8">L2</button><button data-bit="10">L1</button><button data-bit="11">R1</button><button data-bit="9">R2</button></section>
<section class="dpad"><span></span><button data-bit="4">Up</button><span></span>
<button data-bit="7">Left</button><button id="neutral">Neutral</button><button data-bit="5">Right</button>
<span></span><button data-bit="6">Down</button><span></span></section>
<section><button data-bit="0">Select</button><button data-bit="3">Start</button></section>
<section><button data-bit="12">Triangle</button><button data-bit="13">Circle</button><button data-bit="14">Cross</button><button data-bit="15">Square</button></section>
<p id="result">Connecting…</p><p id="diagnostics"></p>
<script>
const result=document.getElementById('result'), diagnostics=document.getElementById('diagnostics');
const held=new Map(); let pending=false, sending=false;
function mask(){let m=65535; for(const b of held.values())m &= ~(1<<Number(b.dataset.bit)); return m;}
async function sendState(){
 pending=true;if(sending)return;sending=true;
 try{while(pending){pending=false;const m=mask();
   const r=await fetch('/state',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'mask='+m,signal:AbortSignal.timeout(700)});
   if(!r.ok)throw new Error('HTTP '+r.status);
   await r.text();result.textContent=m===65535?'Released — ESP32 received':'Held — ESP32 received';
 }}catch(e){held.clear();paint();pending=false;result.textContent='Connection lost — automatic release within 0.8 s';}
 finally{sending=false;}
}
function paint(){document.querySelectorAll('[data-bit]').forEach(b=>b.classList.toggle('held',[...held.values()].includes(b)));}
function releaseAll(){held.clear();paint();sendState();}
document.querySelectorAll('[data-bit]').forEach(b=>{
 b.addEventListener('pointerdown',e=>{e.preventDefault();held.set(e.pointerId,b);b.setPointerCapture(e.pointerId);paint();sendState();});
 const release=e=>{if(held.delete(e.pointerId)){paint();sendState();}};
 b.addEventListener('pointerup',release);b.addEventListener('pointercancel',release);b.addEventListener('lostpointercapture',release);
 b.addEventListener('contextmenu',e=>e.preventDefault());
});
document.getElementById('neutral').addEventListener('click',releaseAll);
window.addEventListener('blur',releaseAll);
document.addEventListener('visibilitychange',()=>{if(document.hidden)releaseAll();});
window.addEventListener('pagehide',()=>{held.clear();navigator.sendBeacon('/state',new URLSearchParams({mask:'65535'}));});
setInterval(()=>{if(held.size)sendState();},200);
async function status(){try{const r=await fetch('/status',{cache:'no-store',signal:AbortSignal.timeout(900)});const s=await r.json();
 diagnostics.textContent='Polled firmware | complete replies: '+s.fullReplies+' | DATA mismatches: '+s.dataMismatches+'\nLast received: '+s.rx+' | DATA readback: '+s.data+'\nHTTP success alone does not confirm console response.';
 }catch(e){diagnostics.textContent='Diagnostics unavailable';}finally{setTimeout(status,1000);}}
sendState();status();
</script></body></html>
)HTML";
