<p align="center">
<img src="https://i.imgur.com/erIpnSF.png" />
</p>

<p align="center">
<a href="https://asciinema.org/a/yQCLhsIRIGNb1IoSfRnbiqYn5?autoplay=0&theme=tango&size=small&cols=76&rows=26&speed=3&loop=1">
<img src="https://i.imgur.com/e8uWUvg.png" />
</a>
</p>

> *"Since they were to come in the days of the power of Melkor, Aulë made the dwarves strong to endure. Therefore they are stone-hard, stubborn, fast in friendship and in enmity, and they suffer toil and hunger and hurt of body more hardily than all other speaking peoples; and they live long, far beyond the span of Men, yet not forever."*    
> --  The Silmarillion

In this **programming game**, your goal is to help a group of dwarves establish a small outpost in a dangerous forest.
The game consists of three stages: Stages A and B serve the purpose of a tutorial explaining how the game works, 
while stage C is the actual game. 

<p align="center">
<img src="https://i.imgur.com/S8PHiOI.png" />
</p>

## How to compile and run the game
Building the game requires `g++` compiler and the development version of `ncurses` library (install them for your system before proceeding). 

To start playing, clone the repository, go to the folder `code` and compile the program `dwarves` with:
```
$ make
```

To run the game with the default parameters:
```
$ ./dwarves
```
<p align="center">
<img src="https://i.imgur.com/Y37lyPe.png" />
</p>

The game window consists of three parts: the map, the information panel, and the game log.   
Keyboard keys `[Q]`, `[P]`, `[S]`, and `[F]` are used to **quit**, **play**, **pause**, **step**, 
or **fast-forward**.

**All dwarves' behavior is programmed in the source code file `bot.cpp`.** 

**As part of the game, you are expected to edit this file to control the dwarves.** *(Don't edit the other files of the game.)*
All the necessary details of programming the dwarves' logic are provided in the next section.

After updating `bot.cpp`, re-compile the program with `make`, and re-run the game. 

## Dwarf's programming interface

### Basics of the file `bot.cpp`
The file contains two functions: `onStart` and `onAction`.
The first of them is called when the game starts, and the second is called when an idle dwarf is choosing their next action.

#### Function `onStart`
```c++
void onStart(int rows, int cols, int num, std::ostream &log);
```
It is called at the beginning of the game and receives the following four arguments:
* `rows` is the number of rows of the map,
* `cols` is the number of columns of the map,
* `num` is the number of dwarves you control,   
* `log` is an output stream resembling `cout`, which can be used for text output in the "Outpost log" 
(at the bottom of the game screen). Don't use `cout` or `printf` for printing or debugging, use `log` instead.

The main purpose of the function `onStart` is to initialize global variables if you need to do so.
For instance, the provided code saves `rows`, `cols`, and `num` as global variables `ROWS`, `COLS`, and `NUM`
respectively, so they can be used later.

#### Funciton `onAction`
```c++
void onAction(Dwarf &dwarf, int day, int hours, int minutes, ostream &log);
```
It is called every time a dwarf is idle and is choosing their next action, the parameters are:
* `dwarf` is a `Dwarf` object, which can inspect the map and schedule actions for the dwarf.
This object is described in more detail in the next section.
* `day` (1+), `hours` (0-23), `minutes` (0-59) is the current time,
* `log` is the output log stream, already explained for the function `onStart`.

<p align="center">
<img src="https://i.imgur.com/i3ywbmD.png" />
</p>

### Object `dwarf`

#### Informational methods:

* `int dwarf.row()` - returns the row coordinate of the dwarf,
* `int dwarf.col()` - returns the column coordinate of the dwarf,
* `Place dwarf.look(int row, int col)` - returns `enum Place` that describe the location at *(row, col)*.
Possible return values include:
    - `EMPTY`
    - `DWARF`
    - `PINE_TREE`
    - `APPLE_TREE`
    - `PUMPKIN`  
    - `FENCE`  
    (This is not an exhaustive list, a few other values appear in stage C of the game.)
* `int dwarf.name()` - returns the name of the dwarf; for convenience, it is an integer number (0, 1, 2, ...) uniquely identifying the dwarf,
* `int dwarf.lumber()` - returns the current amount of lumber the dwarves have.

#### Action-scheduling methods:

![](https://i.imgur.com/qNJuPcA.png)

* `dwarf.start_walk(int row, int col)` - schedules a walking action towards the point *(row, col)*. 
The dwarf is intelligent enough to find a short path towards that location, 
so you just have to tell them where to go. However, if they try to walk there but no path is found, 
or if the path becomes blocked, the dwarf becomes idle.

* `dwarf.start_chop(Dir dir)` - schedules to chop a tree in the direction `dir`, which can assume four possible values:
`EAST`, `WEST`, `NORTH`, or  `SOUTH`. 
If there is a tree or a fence in the adjacent square in that direction, the dwarf will chop it and collect lumber. 
If there is no trees or fences, the dwarf becomes idle. 
* `dwarf.start_build(Dir dir)` - schedules to build a fence in the direction `dir`.
* `dwarf.start_pick(Dir dir)` - schedules to pick an apple or a pumpkin (see more on that in stage C).

![](https://i.imgur.com/YgsCm8I.png)

#### More details on action scheduling:

Note that all the action-scheduling methods have no immediate effect when they are called, 
however the dwarf remember your order and will start performing that action after the function `onAction` ends. 
When they complete the action, or if the action cannot be performed, they become idle and the function `onAction` 
will is called again for them, then you can change their order.

Moreover, you **cannot schedule multiple actions** for a dwarf. 
Every time `onAction` is called, you can schedule only one action, multiple calls to
`start_walk`, `start_chop`, `start_build`, and `start_pick` will be ignored, and only the first of them will be executed by the dwarf.

Let's consider the provided implementation of the function `onAction`:

```c++
void onAction(Dwarf &dwarf, int day, int hours, int minutes, ostream &log) 
{
  // Get current position of the dwarf
  int r = dwarf.row();
  int c = dwarf.col();

  // Look if there is a tree on the right from the dwarf
  if (dwarf.look(r, c+1) == PINE_TREE) {
    // If there is a pine tree, chop it
    log << "Found a tree -- chop" << endl;
    dwarf.start_chop(EAST);
    return;
  }
  else {
    // Otherwise, move to a random location
    int rr = rand() % ROWS;
    int cc = rand() % COLS;
    log << "Walk to " << rr << " " << cc << endl;
    dwarf.start_walk(rr, cc);
    return;
  }
}
```
The above function gets the current dwarf coordinates *(r, c)*, checks if there is a `PINE_TREE` at the coordinates *(r, c+1)*,
which corresponds to the east direction (to the right from the dwarf).
If there is indeed a tree, the dwarf schedules a chopping action in the `EAST` direction.
Otherwise, if there is no pine trees there, it picks a random point on the map and schedules a walking action in that direction.

Notice that in the above code, we do `return;` right after scheduling an action. 
This is not required, but is advised to do, because once an action is scheduled,
the rest of the code in the `onAction` function will not do anything to the dwarf (unless you want to change global state). 
So, it is suggested to return from the function immediately after calling action-scheduling functions 
(`start_walk`, `start_chop`, `start_build`, and `start_pick`).

Obviously, the code shown above is not a very efficient way to chop trees. 
In stage A of the game, your goal will be to improve the dwarf's logic to collect lumber more efficiently.


## Stage A. Collect lumber
<p align="center">
<img src="https://i.imgur.com/Qr2bohh.jpg" />
</p>

At this stage, the game starts with only one dwarf to control (there will be more of them in the following stages).

**Your goal for this stage is to collect 100 lumber in 18 hours** (starting at 6:00 in the morning, until 21:00 at night).
The amount of collected lumber is shown in the top-left corner of the window.
**Improve the dwarves' code (file `bot.cpp`) to achieve this goal.**

To compile your code:
```
$ make
```
To run:
```
$ ./dwarves
```

<details> <summary><b>&#127875; Show Hints </b></summary>

One suggestion for stage A is to implement a helper function
```c++
bool isNextToATree(Dwarf & dwarf, int r, int c);
```
which should return `true` if there is at least one tree adjacent to the location *(r, c)*, that is, there is a `PINE_TREE` or an `APPLE_TREE`
at *(r+1, c)*, *(r-1, c)*, *(r, c+1)*, or *(r, c-1)*. Otherwise, return `false`.

Then the main `onAction` function can work as follows:
- If the dwarf is already adjacent to a tree, they should chop in its direction. 
- Otherwise, you should look for an empty location with trees nearby and walk towards that point. 
(The above function can be quite helpful for this task.)

Feel free to define more helper functions when you feel they can be helpful to express your program in a more concise and clear fashion.

Also, note that the `dwarf.start_walk` action will not work if the destination point is non-empty (blocked by a tree or by another dwarf),
or if no possible path exists.
So, when you start this action, the destination point should be empty. (In this task, aim for empty locations that are adjacent to trees.)
</details>

## Stage B. Build an outpost
<p align="center">
<img src="https://i.imgur.com/XhfC3ky.jpg" />
</p>

In the previous stage, we taught the dwarf how to collect some lumber. Now they are ready to build a shelter, or at least some structure that
might be used as a shelter.

**The goal of this stage is to build a contiguous defensive structure consisting of 30 or more fences.** For example, a rectangle 5x6 would work:
```
# # # # # #
# # # # # #
# # # # # #
# # # # # #
# # # # # #
```
Any irregular shape will work too, as long as it remains contiguous and consists of 30 or more fence pieces:
```
  #
# # # # # #
# #     # # #
  #         #
  #     # # #
  # # # # # #
    # # #   # #
        #
```
However, fences touching diagonally don't count as a contiguous structure. For example, below we see three structures of size 9 each 
(not a single structure of size 27):
```
      # # #
      # # #
      # # # 
# # #       # # #
# # #       # # #
# # #       # # #
```

To play Stage B, run the program with the option `b`:
```
$ ./dwarves b
```
You will see that you get 6 dwarves this time. The `bot.cpp` file works the same way as before, 
and it is called for each of the 6 dwarves, when they need to schedule their next action.

To build a fence section at a given location, 
use action `dwarf.start_build(dir)`, which schedules fence construction in the direction 
`dir` from the current dwarf's location.
(Four possible directions are `NORTH`, `SOUTH`, `EAST`, and `WEST`.)
The cost of one fence is 10 lumber.
If you start building without having enough lumber or the target location not empty, the dwarf will become idle
and no lumber will be spent. You can call function `dwarf.lumber()` to find how much lumber your dwarves have.

<details> <summary><b>&#127875; Show Hints </b></summary>

- One approach is to make the dwarves collect all the necessary lumber first, and once they have enough, start construction.
- Don't hesitate to use some global state if you want to coordinate dwarves' actions.
</details>

## Stage C. Survive seven days
<p align="center">
<img src="https://i.imgur.com/9QKI8PV.jpg" />
</p>

**The goals of this stage** (checked at the end of the 7th day):
- &#127775; All dwarves must survive,
- &#127984; Build a contiguous fence structure of size 30 or more,
- &#127822; Collect at least 1000 apples,
- &#127875; Collect at least 30 pumpkins.

To run the program for Stage C, use option `c`:
```
$ ./dwarves c
```

### Daytime, nighttime, and zombies

When you run the program, it does not stop at 21:00 of the first day. Instead, it will run for 7 days.

Each day is divided into two parts:
- **Daytime** from 6:00 to 21:00, and
- **Nighttime** from 21:00 to 6:00 of the next day.

At nighttime, zombies come. 
There are two types of them: `ZOMBIE` and `PUMPKIN_ZOMBIE`. 
You cannot attack them, but they cannot go through fences, so it can be a good idea to build a shelter. 
In the morning, all zombies quickly die, and pumpkin zombies leave pumpkins that can be picked up. 

Zombies can be identified using the function `dwarf.look(row, col)`. It will return `ZOMBIE` or `PUMPKIN_ZOMBIE` if they are at the location *(row, col)*.

Use the action `dwarf.start_pick(dir)` to pick up a pumpkin. The same command can be used to pick apples from apple trees. 
For these actions to work, you need to stand next to a pumpkin or an apple tree and pick in their direction.

If at any time you will need to remove a fence segment, it can be done with the chopping action, `dwarf.start_chop(dir)`. 
This will give you 10 lumber back, which you can use later to build a new fence.

Make use of the `day`, `hours`, and `minutes` variable, as well as the dwarf name `dwarf.name()` to do more precise control over the dwarves' actions.

### Additional configuration options and deterministic PRNG seed
There is an advanced way to start the game with a custom map size:
```
$ ./dwarves c ROWS COLS
```
Replace `ROWS` and `COLS` in the above command with actual numbers, like this:
```
$ ./dwarves c 27 33
```
Two other optional parameters determine the starting number of dwarves and the PRNG seed:
```
$ ./dwarves c ROWS COLS NUM SEED
```

Specifying the SEED number runs the game deterministically:
the game starts with exactly the same initial conditions and all random
choices made by the dwarves are exactly the same each time you run the program.
This may be very helpful for debugging the dwarves' behavior. For example:
```
$ ./dwarves c 27 31 8 55555 
```
will run the game on the map 27&times;31, with 8 dwarves, and using the random seed 55555.

If the additional parameters are not specified, the game in Stage C starts with:
- `ROWS`: 18-22
- `COLS`: 18-22
- `NUM`: 6-8
- `SEED`: random

### Evaluating performance of your code

The game comes with the script `score.rb`, which evaluates your code's performance,
It gives 25 points for reaching each of the four target goals of Stage C
(over-performing beyond the required limits also gives extra credit points).

To run the script, supply it with the path to the `./dwarves` executable as its first argument, i.e.:
```
$ ./score.rb ./code/dwarves
```

Example report:
```
---------------------------------------------------------
Total Score: 95.9 out of 100.

Stars: 🌟🌟🌟🌟🌟

Dwarves survived (on average):     83.0%  (20.8 / 25 pts)
Largest structure (on average):    29.0   (24.2 / 25 pts)
Apples (on average):              997.8   (24.9 / 25 pts)
Pumpkins (on average):             34.3   (26.0 / 25 pts)
---------------------------------------------------------
```
