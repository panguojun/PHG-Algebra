![phg](https://user-images.githubusercontent.com/8099625/169991127-eddeb6bd-b67b-4359-a68a-14e16da3863d.png)
# PHG: A Mini Programming Language for Geometric Modeling  

PHG (Phage Geometry) is a minimalist programming language inspired by bacteriophages, designed specifically for geometric modeling and scene description. It deeply integrates algebraic operations with geometric expressions through a tree-based structure and custom overloading, enabling efficient description of simple shapes to complex 3D scenes.  


## Core Concepts  
- **Node System**: Everything is a node, supporting inheritance, property definition, and hierarchical nesting.  
- **Geometry-First Design**: Built-in operations for vectors, transformations, and spatial relationships.  
- **Dynamic Calculation**: Property values can be computed in real-time via expressions.  
- **Extension Mechanism**: Supports host language integration and custom operation overloading.  


## Basic Syntax  

### Node Definition & Properties  
```phg
// Define a node with properties
cube {
  type: "cube";
  size: 1, 2, 3;
  pos: (time*2), 0, 0; // Dynamic property
  color: #ff0000;
}

// Inheritance and extension
red_cube {cube} {
  color: #00ff00;
}
```  

### Data Structures  
```phg
// Sequence (ordered hierarchical structure)
<vertex1, vertex2, vertex3> = {vertex1{vertex2{vertex3}}}

// Array (unordered set)
[obj1, obj2, obj3] = {{obj1}{obj2}{obj3}}

// Mixed structure
complex_node {
  children: <cube1{}, cube2{}, [sphere1{}, sphere2{}]>
}
```  

### Control Flow  
```phg
// Conditional statement
?(condition) {
  action1();
} : {
  action2();
}

// Loop statement
@i=0; i<10; i++ {
  create_node{i};
}
```  

### Function Definition  
```phg
// Custom function
$calculate_volume(width, height, depth) {
  $return width * height * depth;
}

// Function call
volume = calculate_volume(1, 2, 3);
```  


## Geometric Modeling Features  

### Coordinate Systems & Transformations  
```phg
// 3D node with position and lighting
point_light {
  type: "light";
  pos: 10, 5, 0;
  rotation: 0, 45, 0;
  intensity: 1.5;
}

// Transformation chain
transform_node {
  translate: 5, 0, 0;
  rotate: 0, 90, 0;
  scale: 2, 2, 2;
  
  child_node {} // Inherits all transformations
}
```  

### Geometric Operations  
```phg
// Boolean union
union {
  type: "union";
  children: [cube1{}, sphere1{}]
}

// Boolean difference
difference {
  type: "difference";
  children: [cube1{}, sphere1{}]
}
```  

### Parametric Geometry  
```phg
// Parametric sphere with radius formula
parametric_sphere {
  radius: 5 + sin(time*0.5);
  segments: 32, 16;
  material: "metal";
}
```  


## System Architecture  

### Parser Layers  
1. **Kernel Layer**:  
   - Basic syntax parser (variables, control flow, functions)  
   - Node syntax parser (tree structure traversal and query)  
   - Element operation layer (overloaded algebra and geometry functions)  

2. **Extension Mechanisms**:  
   - **Element Overloading**: Derive from `var_t` to customize operations (e.g., Boolean logic).  
   - **Resource ID Extension**: Link custom resources (meshes, transforms) via unique IDs.  


### External Integration  
- **Code Integration**: Embed as a module or class in host languages.  
- **DLL Interface**: Cross-language calling for VB, C#, etc.  
- **HTTP Server**: Serve geometry data via JSON for web/Unity clients.  


## Code Structure  
```
PHG/
├── base/
│   ├── element.hpp    # Element classes & operation overloading
│   ├── node.hpp       # Node parser implementation
│   ├── phg.hpp        # Core syntax parser
│   └── phg_head.hpp   # Public headers
├── entity/
│   ├── entity.hpp     # 3D entity definitions
│   ├── nodecalc.hpp   # Node logic operations
│   └── sprite.hpp     # 2D graphics interface
├── net/
│   ├── httplib.hpp    # HTTP library
│   └── server.cc      # HTTP server implementation
└── parsers/
    ├── jsonparser.hpp # JSON parser
    ├── xmlparser.hpp  # XML parser
    └── scene.hpp      # Scene entry class
```  


## Python Integration Example  
```bash
pip install phg_vis
```  

```python
from phg import vis
vis('{md:box 1 2 3 color:#ff0000}')  # Render a red box
```  

PHG balances simplicity with extensibility, making it ideal for rapid prototyping in CAD, game development, and generative design.


