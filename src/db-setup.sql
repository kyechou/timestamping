---
--- Database schema for gRPC timestamps demo
---

CREATE TABLE Timestamps (
	id INT NOT NULL AUTO_INCREMENT,
	uuid VARCHAR(38) NOT NULL,
	name VARCHAR(128) NOT NULL,
	rpc_type VARCHAR(9) NOT NULL,
	peer VARCHAR(128) NOT NULL,
	seq INT,
	msg_size INT,
	sendmsg_time DOUBLE,
	scheduled_time DOUBLE,
	sent_time DOUBLE,
	received_time DOUBLE,
	acked_time DOUBLE,
	PRIMARY KEY (id)
);

