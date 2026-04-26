# Pixel Art Mode — Code Flow Reference

## Two Timers

Two independent timers drive everything, sharing one `secondsTrigger` guard variable in `Pixel.h`:

- **Background** cycles every **13 seconds**
- **Sprites** cycle every **5 seconds**

---

## Background Modes (cycle every 13s)

| Index | Name | Description |
|---|---|---|
| 0 | Corner | Concentric circles radiating from all four corners |
| 1 | Square | Rainbow concentric squares from the center outward |
| 2 | Squares | Four mini rainbow squares, one per quadrant |
| 3 | Plasma | Slow animated psychedelic wave |

---

## Sprite States (cycle every 5s)

Starts at state **4** (alien) on first load.

| State | What shows |
|---|---|
| 0 | Smiley face scrolls across once, then holds off-screen |
| 1 | Nothing — background only |
| 2 | Two space invaders bouncing around |
| 3 | Nothing |
| 4 | Alien face (fills full 16×16 matrix) |
| 5 | Nothing |

Cycle order from startup: **alien → nothing → smiley → nothing → invaders → nothing → alien → ...**

---

## Background Overrides

Two sprites forcibly lock the background every frame, regardless of the timer:

- **Invaders** always force **Plasma**
- **Alien** always forces **Squares**

The background timer only visibly matters during "nothing" states and while the smiley is crossing.

---

## First ~65 Seconds

```
t=0s   Alien    + Squares (alien forces it)
t=5s   Nothing  + Corner  (background timer was at Corner)
t=10s  Smiley   + Corner
t=13s  Smiley   + Square  (background ticks mid-scroll)
t=15s  Nothing  + Square
t=20s  Invaders + Plasma  (forced)
t=25s  Nothing  + Plasma
t=26s  Nothing  + Squares (background ticks)
t=30s  Alien    + Squares (alien forces it anyway)
t=35s  Nothing  + Squares
t=39s  Nothing  + Plasma  (background ticks)
t=40s  Smiley   + Plasma
t=45s  Nothing  + Plasma
t=50s  Invaders + Plasma  (forced)
t=52s  Invaders + Corner  (background ticks, but invaders override to Plasma)
t=55s  Nothing  + Corner
t=60s  Alien    + Squares (alien forces it)
```

---

## Known Quirk — 65-Second Collision

Every **65 seconds** (LCM of 5 and 13), both timers try to fire on the same second. Because they share `secondsTrigger`, the background change wins and the sprite change is skipped. Whichever sprite is active at that moment stays on for **10 seconds** instead of 5.

The full sequence doesn't truly repeat until around **13 minutes** in.
