
import lejos.hardware.*;
import lejos.hardware.motor.EV3LargeRegulatedMotor;
import lejos.robotics.RegulatedMotor;
import lejos.robotics.navigation.DifferentialPilot;


public class robot {
	
	public static void main(String[] args) {
        square(args);
	}
	
	//Draws square
	public static void square(String[] args) {
        Brick brick=BrickFinder.getDefault();
        
        RegulatedMotor leftMotor = new EV3LargeRegulatedMotor(brick.getPort("B"));
        RegulatedMotor rightMotor = new EV3LargeRegulatedMotor(brick.getPort("C"));
        DifferentialPilot pilot = new DifferentialPilot(6.24, 11.55, leftMotor, rightMotor);
        
        for(int i=0; i<4; i++) {
            pilot.travel(40);
            pilot.rotate(90);
        }
	}
 

}
