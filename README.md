# Grafika

Welcome to my Graphic Design assignment. Buckle up boys and girls, silence your phones, and say your goodbyes to loved ones as we’re about to embark on a caffeine-fueled descent into madness, powered by coffee, energy drinks, a deeply concerning number of cigarettes, and enough whiskey to legally classify this project as a cry for help. Turbulence is guaranteed. Sleep is not.

This will be long. It will be messy. At some point, I will zoom in to 6400% and still not know what I’m doing.

Honestly, this quote sums it up perfectly: “I have nothing to offer but blood, toil, tears and sweat.”

There will be phases: confidence, confusion, despair, denial, five minutes of productivity, more despair, breaking down crying, bargaining with the devil, accidentally deleting everything, and finally if the stars align… something that vaguely resembles a finished project.

This adventure officially begins on 21.03.2026. and is due on 01.05.2026.
Or, more realistically, whenever I emotionally recover enough to submit it.



The Game



Need For Weed is a 3D driving game developed in C using OpenGL and FreeGLUT. The project focuses on procedural content generation, real-time interaction and basic phisics simulation. The player controls a car on a dynamically generated track, avoiding obsticled, collecting coins and reacting to environmental effects such as rain or oil spills.



Procedural Environment



* Road generated using Catmull–Rom spline interpolation
* Random placement of trees outside the road area
* Irregularly shaped lakes with shoreline transitions
* Dynamic cloud placement



Vehicle System



* Smooth acceleration, braking, and turning
* Speed-based steering behavior
* Nitro boost functionality
* Third-person camera following system



Collision and Game Logic



* Collision detection with:

&#x20; \* Trees

&#x20; \* Start line poles

&#x20; \* Map boundaries

* Life system with three lives
* Game over triggered by:

&#x20; \* Losing all lives

&#x20; \* Entering water (car sinks)

* Sound effectt on collision



Environmental Effects



Rain Zones



* Randomly generated zones aligned with the road
* Wet surface rendering
* Reduced traction affecting vehicle handling



Oil Patches



* Placed directly on the road surface
* Trigger temporary loss of steering control
* Visual feedback with blinking on-screen warning



Coins and Scoring



* Coins placed along the center of the road
* Random spacing between coins
* Score increases upon collection
* Sound feedback on pickup
* Coins regenerate after crossing the start line



User Interface



* On-screen life indicator using animated heart icons
* Visual feedback for:

&#x20; \* Collisions (screen flash)

&#x20; \* Oil effect (blinking warning)

* In-game menu system:

&#x20; \* Description

&#x20; \* Controls

&#x20; \* Credits

&#x20; \* Exit

* Game over screen with restart option



Technical Details



* Language: C
* Graphics: OpenGL with FreeGLUT
* Rendering:

&#x20; \* Immediate mode (glBegin / glEnd)

&#x20; \* 2D overlays for UI elements

* Procedural systems:

&#x20; \* Road generation via spline sampling

&#x20; \* Random object distribution with spatial constraints

* Physics:

&#x20; \* Basic acceleration and friction model

&#x20; \* Context-sensitive handling (rain, oil)



Requirements



* C compiler (GCC / MinGW recommended)
* OpenGL
* FreeGLUT
* Windows (required for sound playback via PlaySound API)



Notes



This project was developed as part of a computer graphics coursework. The primary goal was to implement a functional 3D environment with interactive gameplay elements and procedural generation techniques.



