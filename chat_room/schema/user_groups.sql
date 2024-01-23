user_groups(user_id INT,
		    user_name VARCHAR(255),
		    group_id INT,
		    group_name VARCHAR(255),
		    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
		    FOREIGN KEY (group_id) REFERENCES group_lists(id) ON DELETE CASCADE)