group_lists(id INT AUTO_INCREMENT PRIMARY KEY, 
		    group_name VARCHAR(255) UNIQUE, 
		    owner_id INT,
		    owner VARCHAR(255),
		    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE)
