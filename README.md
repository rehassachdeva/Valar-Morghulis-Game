####Please run make or directly execute the executable 'main'

###Dependencies
* openal
* alut
* GL
* glfw
* ftgl
* SOIL
* dl
* freetype2

###Features and controls

####Views

* Tower View from about 8 angles which can be switched with key 'r'
* Top view
* Adventure View
* Follow-cam View
* Helicopter-cam view with ability to rotate angle of view with left mouse click and drag.
* The views can be switched with 'v'
* Zoom in zoom out ability with mouse scroll
* Key 'l' lights up or switches off the light of the course depending upon the direction the player facing.

####Obstacle course
* Spheres are obstacles, cannot be jumped over, collision with them makes you lose life
* Blocks moving up and down can only be crossed with jump. But if collision happens during jump then you lose a life.
* Empty blocks means you simply sink down and lose a life
* Diamonds carry points
* Goal is to reach the diagonally opposite block from start, to acquire the dragon.
* Player can also fall down from the edges of the grid and lose a life.
* Various animations are implemented to help you know when you score, lose a life, win etc.
* Each time you lose a life you are reset to the starting position
* Timer indicates elapsed time from start of a game
* A game carries 3 lives
* Squares with your player's kingdom image periodically grant extra lives with some probability.
* Unlimited levels in increasing order of difficulty (number of missing blocks, blocks moving up and down)
* Music to support the theme of Valar Morghulis and goal of acquiring the dragon :P

####Movement Controls
* Speed of movement can be increased or decreased using 'f' and 's' respectively.
* Move up using 'UP' arrow key
* Move down using 'DOWN' arrow key
* Move left using 'LEFT' arrow key
* Move right using 'RIGHT' arrow key.
* Jump up is carried using 'space'+'up arrow key' 
* Jump down is carried using 'space'+'down arrow key' 
* Jump left is carried using 'space'+'left arrow key' 
* Jump right is carried using 'space'+'right arrow key' 

