# Work Distribution — Zero Point
**Project:** Zero Point – Where Forest Meets Village  
**Course:** CSE422 — Computer Graphics Lab  
**Team Size:** 5 | **Duration:** 2 Weeks

---

## Part 1 — Quick Overview

| Person | Zone / Focus | Core Deliverables |
|---|---|---|---|
| **Person 1** | River + Birds | Bresenham Line algorithm, `drawRiver()`, `drawBirds()`, birds animation |
| **Person 2** | Forest Zone | `drawForest()` (trees + grass), `drawFireflies()`, fireflies animation |
| **Person 3** | Village | `drawVillage()` (house + windmill + grass), windmill rotation |
| **Person 4** | Fence + Cat + Deer | DDA Line algorithm, `drawFence()`, `drawCat()`, `drawDeer()`, cat animation, deer animation |
| **Person 5** | Sky Zone + System | Midpoint Circle algorithm, `drawSky()` (full ownership), auto day/night timer, `display()` |

---

## Part 2 — Detailed Descriptions

---

### Person 1 — River & Birds

**Zone:** The diagonal river that splits the entire scene into forest and village, and the birds drifting across the sky during the day.

**Week 1 — Algorithm & Drawing:**
- Implement `drawBresenhamLine()` from scratch. This is the core rasterization algorithm that draws both edges of the diagonal river. The river runs diagonally from top-right to bottom-left — Bresenham handles this accurately using incremental error logic. Understand and implement the algorithm yourself.
- Implement `drawRiver()` using the two Bresenham edges as boundaries. Compute the two parallel diagonal edge lines, then fill the strip between them as a solid blue polygon to form the river body. The river crosses the 80% ground zone diagonally.
- Implement `drawBirds()` — birds are small V-shapes drawn using `GL_LINES`. Draw 2–3 birds at different x and y positions in the sky. Day only; birds must not appear at night.

**Week 2 — Animation:**
- Animate the birds with slow left-to-right translation across the sky. Each frame, increment the x position of all birds by a small amount. When a bird moves past the right edge of the screen, reset its x position back to just off the left edge so it loops continuously. This is the **Translation** transformation for this person's work.

**What the teacher sees from you:** Bresenham Line algorithm implemented from scratch, the diagonal river drawn with it, and bird translation animation.

---

### Person 2 — Forest Zone

**Zone:** The upper-left triangle — the wild, dense side of the scene. This is the most visually complex single zone in the project.

**Week 1 — Drawing:**
- Implement `drawForest()`. This is the heaviest drawing function in the project and includes:
  - Draw 3–4 trees using rectangles for trunks and triangles or rounded canopy shapes. Intentionally draw them at **different sizes** to create a sense of depth — a near tree is large, a far tree is small. This is the **Scaling** transformation demonstrated through the scene.
  - Draw tall grass using dense clusters of `GL_LINES` in dark green tones along the river edge and between trees. Grass lines should be noticeably taller and denser than village grass.
- Implement `drawFireflies()` — fireflies are rendered as `GL_POINTS` in bright yellow-green, scattered across the forest bounding box. Place around 10–15 firefly points at varied positions. Night only.

**Week 2 — Animation:**
- Animate the fireflies by applying a small random offset to each firefly's x and y position every frame. The offset should be tiny (±1–2 pixels) so they appear to gently drift and flicker rather than jump. Keep them within the forest bounding box — if one drifts out of bounds, nudge it back in. Night only.

**What the teacher sees from you:** The most complex drawing zone with scaling applied across trees, and the firefly particle animation system.

---

### Person 3 — Village Structures & Deer

**Zone:** The lower-right triangle — the domestic side. This person owns the village buildings and the windmill rotation.

**Week 1 — Drawing:**
- Implement `drawVillage()`. This includes:
  - Draw the house using rectangles for walls and a triangle for the roof. Add a small rectangle for the door. Use brown tones as in the reference image.
  - Draw the windmill — a tall rectangle or trapezoid for the body with a small window and door. Draw the windmill blades as four elongated thin rectangles arranged in an X pattern around a center pivot point. The blades and body must be drawn separately so the blades can be rotated independently in Week 2.
  - Draw short grass around the village area using `GL_LINES` in lighter green, shorter than forest grass.

**Week 2 — Animation:**
- Implement windmill blade rotation using `glRotatef`. The blades must rotate around the windmill's center pivot point. Use `glPushMatrix` before applying the rotation and `glPopMatrix` after — this isolates the blade rotation so the windmill body stays still. Increment the rotation angle each frame. The windmill rotates **always**, both day and night. This is the **Rotation** transformation.

**What the teacher sees from you:** Village buildings and windmill rotation transformation (glPushMatrix/glPopMatrix).

---

### Person 4 — Fence & Cat

**Zone:** The fence running along the village side of the river, the cat that wanders near it during the day, and the deer that visits the river at night.

- Implement `drawDDALine()` from scratch. DDA (Digital Differential Analyzer) is the second required line-drawing algorithm in the project. It calculates intermediate points along a line using floating-point increments. Implement the step-by-step DDA logic yourself.
- Implement `drawFence()` using `drawDDALine()`. The fence runs diagonally along the village edge of the river, parallel to the river direction. Draw evenly spaced vertical fence posts using DDA, and connect them with horizontal rail lines. The fence visually reinforces the boundary between river and village.
- Implement `drawCat()` — the cat is orange and sits near the fence. Build it from simple shapes: an oval or rectangle body, a circle head, two triangle ears, and a tail using a bent `GL_LINE_STRIP`. Day only.
- Implement `drawDeer()` — the deer appears on the forest side near the river edge at night. Build it from basic shapes: a rectangle body, a smaller rectangle or circle for the head, line legs, and two short forked lines for antlers. Night only.

- Animate the cat with ping-pong translation along the village side of the river edge. The cat moves left and right within a short range. When it hits either boundary, the direction flips. Day: visible and moving. Night: hidden. This is the **Translation** transformation for this person's work.
- Animate the deer with ping-pong translation along the forest edge of the river. The deer moves left and right within a bounded range. When it reaches either boundary limit, direction reverses. Day: hidden. Night: visible and moving. This is the **Translation** transformation for this person's work.

**What the teacher sees from you:** DDA Line algorithm implemented from scratch, the fence drawn using it, the cat figure with translation animation, and the deer figure with translation animation.

---

### Person 5 — Sky Zone & System

**Zone:** The top 20% of the screen (sky), and the shared engine behind the whole project — the day/night flag, the animation update loop, and the master render pipeline.

**Week 1 — Algorithm & Drawing:**
- Fully own `drawSky()`. The sky background changes between light blue (day) and deep navy (night). The base version is already implemented — Week 1 is about completing and polishing it: refine the clouds (overlapping white rounded quads, day only) and ensure the star field (`GL_POINTS`) looks natural at night.
- Fully own and implement the **Midpoint Circle algorithm** (`drawMidpointCircle()`). This is the third required algorithm. It is used for the sun (day, filled yellow circle) and the moon (night, white crescent achieved by drawing two offset circles and layering the background color). The algorithm is partially implemented — take full ownership of it and be able to explain every line.
- Ensure the keyboard shortcut handler for manually toggling `isNight` is clean and working.

**Week 2 — System:**
- Implement the automatic day/night timer inside `update()` (the GLUT idle callback). A global frame counter increments every frame. When it reaches ~500, flip the `isNight` boolean and reset the counter. This single boolean drives the entire scene — every other person's draw functions read it to decide what to show or hide.
- Implement `display()` — the master rendering function that calls every draw function in the correct order: `drawSky()` → `drawRiver()` → `drawForest()` → `drawVillage()` → `drawFence()` → `drawDeer()` → `drawCat()` → `drawFireflies()` → `drawBirds()`. Order matters in OpenGL because elements drawn later appear on top. Getting this order right is what makes the final scene look correct.

**What the teacher sees from you:** Midpoint Circle algorithm (sun + moon), complete polished sky zone, the auto timer system, and the display pipeline that assembles every person's work into the final scene.

---


## Timeline Suggestion

| Week | Focus |
|---|---|
| **Week 1** | Everyone implements their algorithms and static drawing functions. Goal: full scene visible in day mode with no animations yet. Person 5 sets up `display()` and the `isNight` flag early so everyone can plug in and test their draw functions independently. |
| **Week 2** | Everyone adds their animations. Person 5 wires up the auto timer. Final pass: test day/night toggle and verify each person's elements correctly appear and disappear as expected. |