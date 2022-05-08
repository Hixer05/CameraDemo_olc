# olcPGE_demos

---

## What's this?

These are demos I created while playing around with Javidx9's PixelGameEngine.

### Gravity Simulation

In this Demo I have included a gravity simulation between objects, using classical physics. You can try it out and play around, press I to switch to insert mode where you can create objects of various masses, then press the left mouse button and drag the mouse to give the object some velocity.
I've also included with camera and scaling (zooming), move with WASD; and zoom with UP and DOWN.

![gs2](https://user-images.githubusercontent.com/60942163/156193181-875162c8-00c5-4ae6-ad9d-01119ca04e5b.gif)

#### Gravity Refactored

In this demo I re-implemented the gravity simulation ( partially ) with an additional thread that does all of the computation freely from the main one, which draws. Generally it's 100 frames ahead of the drawing process.

##### Problems with Gravity

I suspect that the simulations are wrong, and I am currently revising them. It may be the physiscs values I use, or it may be a problem with how I handle variables. Feel free to point  out errors or better methods in the issues section.
