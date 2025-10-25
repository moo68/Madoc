# Madoc - A World Generator
Named after the legendary Welsh king who supposedly discovered the Americas centuries before Columbus, Madoc aims to be a semi-realistic random world generator capable of creating continents, oceans, and terrain with various different biomes. It's inspired by things like Azgaar's Fantasy Map Generator, Paradox grand strategy games, and my own personal interest in fantasy worldbuilding.

Madoc is also a personal project of mine to get even more familiar with OpenGL and C++. I've realized I have a passion for low-level programming--especially graphics programming. Thus, I'm making this project as from-scratch as possible.

Done in collaboration with a mentor through the LaunchPad club at Purdue. Thank you Andrew!

See /external for the different external libraries vendored in, such as GLAD and GLFW.
## To-Do
- OpenGL boilerplate [DONE]
- Voronoi cell generation logic [DONE]
- Bitmask logic for a specific Voronoi cell [DONE]
- Converting Voronoi cells into veritces [DONE]*
- Triangularizing Voronoi cells (triangle fan) [DONE]
- Properly rendering a full grid of Voronoi cells
- Triangularizing Voronoi cells (ear clipping)
- Changing Voronoi cell attributes based on Perlin noise
- ...and more

*This might be something that needs to get looked at again; still fully functional, though
