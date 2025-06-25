import time
from gcu_api import VisPHG

class RocketAnimation:
    def __init__(self):
        # Animation parameters
        self.frame_interval = 0.05  # 20 FPS
        self.launch_speed = 0.2     # units per frame
        self.rotation_speed = 1    # degrees per frame
        self.current_height = 0
        self.current_angle = 0
        self.stage_separated = False
        self.separation_height = 15
        self.max_height = 30
        self.running = False
        
        # Rocket parameters
        self.body_radius = 1.0
        self.body_height = 8.0
        self.nose_height = 2.0
        self.engine_length = 0.8
        
    def generate_rocket_script(self):
        """Generate the VisPHG script for the current frame"""
        main_body = f"""
        # Main rocket body
        body{{md:cylinder {self.body_radius} {self.body_height}; 
              x:0.0;y:{self.current_height};z:0.0;
              rx:0.0;ry:{self.current_angle};rz:0.0;
              sx:1.0;sy:1.0;sz:1.0}}
        
        # Nose cone
        nose{{md:sphericalcrown 1.2 1.0 {self.nose_height}; 
              x:0.0;y:{self.current_height + self.body_height};z:0.0;
              rx:0.0;ry:{self.current_angle};rz:0.0;
              sx:1.0;sy:1.0;sz:1.0}}
        """
        
        # First stage (disappears after separation)
        first_stage = ""
        if not self.stage_separated:
            first_stage = f"""
            # First stage engine
            engine{{md:cylinder 0.6 {self.engine_length}; 
                   x:0.0;y:{self.current_height - self.engine_length};z:0.0;
                   rx:3.14;ry:{self.current_angle};rz:0.0;
                   sx:1.0;sy:1.0;sz:1.0}}
            
            # Fins
            fin1{{md:cylinder 0.1 2.0; 
                  x:0.0;y:{self.current_height};z:-1.73;
                  rx:1.57;ry:{self.current_angle};rz:0.0;
                  sx:1.0;sy:0.3;sz:1.0}}
            fin2{{md:cylinder 0.1 2.0; 
                  x:1.5;y:{self.current_height};z:0.87;
                  rx:1.57;ry:{self.current_angle};rz:2.09;
                  sx:1.0;sy:0.3;sz:1.0}}
            fin3{{md:cylinder 0.1 2.0; 
                  x:-1.5;y:{self.current_height};z:0.87;
                  rx:1.57;ry:{self.current_angle};rz:-2.09;
                  sx:1.0;sy:0.3;sz:1.0}}
            """
        
        # Window (always visible)
        window = f"""
        # Window
        window{{md:sphericalcrown 0.3 0.3 0.3; 
                x:0.0;y:{self.current_height + 5.0};z:1.05;
                rx:0.0;ry:{self.current_angle};rz:90.0;
                sx:1.0;sy:1.0;sz:1.0}}
        """
        
        return f"""
        {{ry:{self.current_angle*2};
            {main_body}
            {first_stage}
            {window}
        }}
        """
    
    def update_frame(self):
        """Update animation state for next frame"""
        self.current_height += self.launch_speed
        self.current_angle = (self.current_angle + self.rotation_speed) % 360
        
        # Check for stage separation
        if not self.stage_separated and self.current_height >= self.separation_height:
            self.stage_separated = True
            # Reduce body length after separation
            self.body_height = 5.0
            # Increase speed after stage separation
            self.launch_speed = 0.3
    
    def run_animation(self):
        """Run the complete launch animation"""
        self.running = True
        
        # Initialization
        VisPHG("""
        echo(0);
        """)
        
        # Animation loop
        while self.running and self.current_height < self.max_height:
            frame_start = time.time()
            
            # Generate and send the frame
            script = self.generate_rocket_script()
            VisPHG(f"""
            clrsm();
            {script}
            setup();draw();
            """)
            
            # Update for next frame
            self.update_frame()
            
            # Maintain frame rate
            elapsed = time.time() - frame_start
            time.sleep(max(0, self.frame_interval - elapsed))
        
        # Final frame
        VisPHG("clrsm();")
        self.running = False

if __name__ == "__main__":
    rocket = RocketAnimation()
    
    # Customize parameters if needed
    rocket.launch_speed = 0.15     # Slower initial ascent
    rocket.rotation_speed = 1.8    # Slower rotation
    rocket.separation_height = 20  # Higher separation point
    rocket.max_height = 40         # Higher final altitude
    
    # Run animation
    rocket.run_animation()
