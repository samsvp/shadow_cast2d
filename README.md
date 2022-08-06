# Shadow Cast 2D

Shadow caster 2D based on [javidx9](https://www.youtube.com/watch?v=fc3nnG2CG8U&t=50s&ab_channel=javidx9)
C++ implementation. For more information behind the math check out [redblobgames article](https://www.redblobgames.com/articles/visibility/)
and/or [Nicky Case's](https://ncase.me/sight-and-light/) article.


## Controls
Use WASD to move and press the right mouse button to cast light.

## Dependencies
Build and install [raylib](https://github.com/raysan5/raylib).

## Install
```
mkdir build && cd build
cmake ..
make
```

You should see the shadow_cast binary in the build folder.