	mails(id INT AUTO_INCREMENT PRIMARY KEY,
	      receiver_id INT,
	      receiver VARCHAR(255),
	      sender_id INT,
	      sender VARCHAR(255),
	      messages VARCHAR(8192),
	      FOREIGN KEY (receiver_id) REFERENCES users(id) ON DELETE CASCADE)