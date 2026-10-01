# Reply to ANTS PCB

Subject: ส่งไฟล์แก้ไข NewPlayBridge2 V4.1 สำหรับตรวจสอบอีกครั้ง

เรียน ทีมงาน ANTS PCB

ขอบคุณสำหรับคำแนะนำครับ/ค่ะ ได้แก้ไขไฟล์ NewPlayBridge2 เป็นเวอร์ชัน V4.1 แล้ว โดยปรับ VIA ที่ใช้เดินลายทั้งหมดดังนี้

- Hole size ของ VIA: 0.40 mm (16 mil)
- Diameter / PAD ของ VIA: 0.90 mm (36 mil)
- Annular ring: 0.25 mm ต่อด้าน
- จำนวน routed VIA: 190 จุด

ได้ตรวจสอบไฟล์ใหม่แล้ว ไม่พบ DRC error หรือ unconnected item และไฟล์ drill report แสดง VIA ขนาด 0.40 mm จำนวน 190 จุด

แนบไฟล์ `NewPlayBridge2-v4.1-ANTS-Gerbers.zip` เพื่อขอให้ตรวจสอบและใช้แทนไฟล์เดิมครับ/ค่ะ

ก่อนเริ่มผลิต รบกวนช่วยยืนยันเพิ่มเติมว่าสเปกโรงงานรองรับ copper spacing ขั้นต่ำ 0.08 mm และ track width ขั้นต่ำ 0.15 mm สำหรับไฟล์นี้ หากไม่รองรับ กรุณาแจ้งค่าขั้นต่ำที่ต้องการเพื่อให้แก้ไขไฟล์อีกครั้ง

ขอบคุณครับ/ค่ะ

---

English reference:

Thank you for the fabrication feedback. The NewPlayBridge2 design has been updated to V4.1. All routed vias now use a 0.40 mm (16 mil) finished drill and a 0.90 mm (36 mil) copper diameter, giving a 0.25 mm annular ring per side. The regenerated drill report contains 190 routed 0.40 mm via holes, and the updated board passes DRC with no unconnected items.

Please inspect and use the attached `NewPlayBridge2-v4.1-ANTS-Gerbers.zip` instead of the previous file. Before production, please also confirm that your process supports the design's 0.08 mm minimum copper spacing and 0.15 mm minimum track width. If not, please advise the required minimums so the files can be revised.
