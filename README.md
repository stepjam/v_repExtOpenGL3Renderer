# v_repExtOpenGL3Renderer
V-REP external renderer plugin that uses OpenGL 3+ and supports shadows.

There are a number of things to be aware of when including lighting in your scene using this renderer:

**Light parameters** (such as attenuation, spotlight exponent, etc) will have visibly different effects than the default renderer (pre OpenGL3), and so you may want to adjust your values accordingly. This is to be expected given that the way lighting is managed is dramatically different in OpenGL 3+. 

**Shadows** increase rendering time, and so decisions have been made to balance performance vs. render quality. You should be aware that:
- If the light source is too far away, then shadows will not be rendered (i.e. outside of the light's frustum).
- If the angle between the light ray and the surface normal is large, [shadow acne](http://www.opengl-tutorial.org/assets/images/tuto-16-shadow-mapping/ShadowAcne.png) may occur. This can generally be solved by playing with the light location. 
