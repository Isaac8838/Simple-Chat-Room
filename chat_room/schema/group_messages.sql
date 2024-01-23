group_messages( 	id INT AUTO_INCREMENT PRIMARY KEY, 
				user_id INT,
				group_id INT,
				user_name VARCHAR(255), 
				message VARCHAR(8192),
				FOREIGN KEY (group_id) REFERENCES group_lists(id) ON DELETE CASCADE)