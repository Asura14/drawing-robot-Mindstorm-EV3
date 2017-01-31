

import java.io.IOException;
import java.net.ServerSocket;

import lejos.hardware.Button;

public class IsEscapeDownChecker extends Thread {
	ServerSocket socket;
	
	public IsEscapeDownChecker(ServerSocket serversocket) {
		socket = serversocket;
	}
	
	public void run() {
		while (true) {
			
			if (Button.ESCAPE.isDown()) {
				try {
					socket.close();
				} catch (IOException e) {
					System.out.println(e.getMessage());
				}
			}
			
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				System.out.println(e.getMessage());
			}
		}
	}

}
