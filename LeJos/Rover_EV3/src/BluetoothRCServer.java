

import java.io.DataInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import lejos.hardware.Sound;
import lejos.hardware.motor.EV3LargeRegulatedMotor;
import lejos.hardware.port.MotorPort; 



public class BluetoothRCServer {
	public static void main(String args[]) throws IOException {
		int input;
		ServerSocket server = new ServerSocket(1111);
		EV3LargeRegulatedMotor motorA = new EV3LargeRegulatedMotor(MotorPort.B);
		// motorA.setSpeed(5);
		motorA.setAcceleration(75);
		EV3LargeRegulatedMotor motorB = new EV3LargeRegulatedMotor(MotorPort.C);

		// motorB.setSpeed(5);
		motorB.setAcceleration(75);
		
		IsEscapeDownChecker isEscapeDown = new IsEscapeDownChecker(server);
		isEscapeDown.setDaemon(true);
		isEscapeDown.start();
		
		while (true) {
			Socket socket;
			try {
				socket = server.accept();
			} catch (IOException e) {
				break;
			}
			DataInputStream in = new DataInputStream(socket.getInputStream());
			input = in.readInt();
			
			if (input == 1) { // forward
				// motorA.forward();
				// motorB.forward();
				// motorA.setSpeed(100);
				// motorB.setSpeed(100);
				
				motorA.rotate(10, true);
				motorB.rotate(10, true);
			} 
			
			if (input == 2) { // reverse
				// motorA.backward();
				// motorB.backward();
				motorA.rotate(-10, true);
				motorB.rotate(-10, true);
			}
			
			if (input == 3) { // right
				// motorA.backward();
				// motorB.forward();

				motorA.rotate(-5, true);
				motorB.rotate(5, true);
			}
			
			if (input == 4) { // left
				//motorA.forward();
				//motorB.backward();
				
				motorA.rotate(5, true);
				motorB.rotate(-5, true);
			}
			
			if (input == 5) { // stop
				motorA.stop(true);
				motorB.stop(true);
			}
			
			if (input == 6) { // exit & close socket
				Sound.setVolume(50);
				Sound.buzz();
				server.close();
				motorA.close();
				motorB.close();
				System.exit(0);
			}
			
			if (input == 7) { // honk
				Sound.setVolume(100);
				Sound.beep();
			}
		}
		
		Sound.setVolume(100);
		Sound.buzz();
		server.close();
		motorA.close();
		motorB.close();
		System.exit(0);
	}
}
