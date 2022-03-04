=====================================
CIS 460 Final Project: Mini-Minecraft
=====================================
Controls:
F: toggle flight mode (default set to true) (no collision detection during flight mode)
W,A,S,D: for forward, left, back and right movement.
E,Q: for up and down movement in flight mode.
T: toggle 3rd person view (Polar camera controlled by mouse)
Y: Activate Bear's Player-tracking AI when within range
U: Activate Bird's Player-tracking AI when within range

Instructions:
Use QT Creator to run the .pro file on your system to enjoy the full experience!

CREDIT (3 Checkpoints across the entire project):

Checkpoint 1: https://vimeo.com/645386677

Helena: Efficient terrain rendering and chunking

Optimized VBO generation by generating a set of VBO per chunk for every chunk needed in the currently generated terrain. Created functions in the chunk class that can effectively check over neighboring chunks and neighboring blocks to determine if faces need to be generated. Also modified how terrain is generated: from determined whether a zone has been generated, generating each chunk, creating the VBOs for each chunk generated, and drawing the chunks. Seeked help to optimize the order of generating new zones.

The most challenging portion of the milestone is working with the interleaved VBO and creating and sending the VBOs across different files. Since there were many files that needed to be modified to account for the new set of data, it was hard to keep track of which files I still needed to modify while keeping the files compatible with other shaders. 

Mia: Procedural Terrrain

Implemented Worley Noise for the mountain terrain, and fbm - perlin for the grass terrain. Generally, what I found the most challenging about my role was that there is a lot of experimenting involved and it is difficult to tell what will happen after tweaking certain values. Therefore it was a significant amount of trial and error, and it required a lot of patience.

More specifically, for the Worley Noise, I was having a lot of trouble dealing with discontinuities in the terrain, as the render would show random sections where the height of the terrain would suddenly drop or elevate. On the other hand, for the FBM - Perlin noise, I kept seeing that the Perlin noise function would consistently output a noise value of 0.0, and it took me a while to realize that the reason was due to the fact that the input uv coordinates have to be scaled by large denominators.

Logan: Player Physics and Tick Function:

Implemented controls and input bundles through keypress and keyrelease event functions in MyGL- I added a release function in order to allow for the player to hold onto keys and consistently apply acceleration in some direction, as well as allow for multi-directional movement since multiple keys could be indicated as true in a bundle.

As for mouse movement for rotation, I bound right-local rotation (y-axis on screen) at the poles and mapped the displacement from the center of the widget to angles to be used for the polar rotations of the camera.

Implemented player/terrain collisions through ray casting velocity * dT/1000 from every vertex of the volume of the player (1x2x1 block), split into the cardinal axes each time to allow for smoother gameplay. Iterated through a vector of vertices, and for each grid marched + and if collision occured, reduced displacement in that specific axis by a proportion of the distance until intersection, allowing for smoother velocity adjustment.

Faced a difficulty in grid marching, and eventually overcame this difficulty by tweaking the lecture provided grid marching algorithm, specifically with the addition of an if statement before checking if a non empty block is present at some current coordinate which would prevent the algorithm from ray casting beyond the maximum length (length of the ray cast.)

Checkpoint 2: https://vimeo.com/manage/videos/648454727

Helena: Multithreading

Created the BlockTypeWorker and VBOWorker class to use multithreading to generate terrain and create VBO data. Some functionalities were moved from terrain and chunk classes into the worker classes. Also used Mutex to ensure different threads do not edit the same information at the same time. One of the biggest challenges would be checking when new zones need to be generated or destroyed, since a location for previous location needs to be passed between myGL and terrain to keep track of the changes. 

Logan: Texturing

Implemented OpenGl Texturing and improved Player physics (added auto jump for blocks of height 1).

Used lecture notes and HW 4 as my main resources for texturing and UV coordinate revision. Used a switch case to send different UV coords for different kind of blocks. Had a lot of difficulty with splitting the VBO into two different ones for Opaque and Transparency, but after merging with my teammates multithreading, had a better understanding and managed to work it out. 

For animation, I made a uniform time variable that is incremented every tick, and using this, for each of the rows of 3 of lava and water tiles in the texture, I had the UV's shift slightly in a loop dependent on time. Made UV coordinates a vec3 with the final value being a 0 or 1 depending on whether the UV should be animated or not! (I.e. an animateFlag)

Mia: Cave Systems

Using the class slides, I added a new function to our Biome class, perlinNoise3D. Initially I tested the noise function out for y coordinates in the range [64, 128] so that it would be easier to see the results of adjusting the denominator for the (x, y, z) coordinate input. Since the cave system implementation causes the program to slow down a little, we discussed as a group that a potential solution was to use 2 different noise functions which would determine the y values for the bottom y value and top y value of a cave opening for each (x, z) coordinate.

For the shaders that are applied when the players is submerged in lava and water, I reviewed the HW04 materials and used it as an example. I added 2 new post process shaders which interpolate between pure red/blue and the input color. A challenge we encountered was that the shaders wouldn't activate/deactivate exactly when the player's eyeline was crossing the liquid surface - we did some fiddling in the player class to fix this problem.

Checkpoint 3: Link to final video: https://youtu.be/rdcXJfIWjBc

Helena: Procedural Asset Placement (Flower, Mushroom, Giant Trees, Cacti, Giant Cacti), L-System Shape Grammar Structures (Giant Cacti), Player Inventory GUI

Using the existing noise functions used for biomes, I used them to procedurally place small and large asset throughout the world depending on the biome and surrounding block types. For generating the giant trees, because they can span up to 15 by 15 blocks, they might reach into other chunks, so I had to have that in mind and pass in a 3 by 3 area of chunks to generate the trees correctly. The leaves of the tree are also placed depending on if there are other trees near by. For L-System Cacti, I created the L-System grammar and matched each symbol to a step in creating the cactus. The player inventory GUI is generated in a similar way as the crosshair, where I used the player's view matrix to ensure that the GUI does not turn and warp as the player moves and turns around.

The most challenging portion of the assignment is generating the grammar for the L-System Cacti. To make sure that the linked list nodes do not get caught in an infinite loop, I had to make sure that each node is a different pointer, and it was difficult to keep track in my parse function which pointers need to be created and which pointers are the original pointers. 

Logan: Third-Person Mode, NPC and AI (Pathfinder algorithm)

Wanting to easily and fully animate my models, I opted for a scene graph implementation similar to HW2, but instead with 3D cuboids (called MobCuboid in our code.) I used this scene graph implementation to create three different models, first the player, next a Bear NPC, and finally a Bird NPC. For animating the models, I used sin and cos functions to simulate SHM (swaying) of limbs such as arms/legs while running, and updated the model's position to be drawn at via. updating the model's highest translation node to be some function of the entities' physical position. Implementing running, idle and hitting animations was really difficult and required lots of pre-thought and planning to ensure it worked. 

For the camera aspect of third-person mode, I decided to have two cameras (a first person camera and a third person camera) as member variables of player, and to simply toggle mcr_camera between the two cameras as needed. The third person camera was simply the first person camera's attributes, but translated by some multiplier backwards in the direction of the first person camera's forward vector, ensuring that the camera was always behind the player.

As for the AI of my mobs (the bear and the bird), I implemented greedy BFS that is active when the player is within some set maximum distance of the mob. The algorithm constructs a 2D array graph of the mob's surroundings, placing non-negative weights to locations that are possible to reach, and if the player's tile is able to be reached, an optimal* path is constructed from the mob to the player, and the mob then follows this path of movements. 

1. I used a deque for storing these paths as it allowed me to iterate from the player's position on the grid backwards, while constructing a path in the correct order (i.e., something I would have otherwise had to have used a stack for). Additionally, I used my pre-existing Player physics to support the mob's movement, adding an additional enum of possible movements (LEFT, RIGHT, FORWARDS, BACKWARDS), that represent movements in those world directions for exactly one tile, thus allowing mobs to accurately follow paths of width >= 1.

*While greedy BFS is not the most optimal in every single scenario, the algorithm allows for mobs to avoid impassable walls and ravines and take a relatively shortest path.

Mia: Additional Biomes, Procedural Assets (Corals, Grass)

I created 5 additional biomes to supplement the biomes that already existed in milestone 2. For the iceberg terrain, I mirrored the Ice Mountain shape below water to create iceberg-like structures, used smoothstep on perlin and FBM noise for the plateau biome, a fixed y coordinate for the flat desert biome, fixed a water level for the swamp biome, and tweaked FBM values for the ocean biome. I used a vec3 of perlin noise values for the biome interpolation. Furthermore, I used noise functions to procedurally place the coral spread in the ocean biome, and the grass patches in the swamp biome.

What I found the most challenging was adjusting the scaling denominator and offset values for all the noise functions which include perlin noise, FBM noise, smoothstep, and more. It was a lot of experimentation, and often it would be hours until I reached a point where I felt that I had some grasp on how to make the biome look convincing. Overall I am very happy with the result of the 8 biomes as it makes the game setting exciting.

