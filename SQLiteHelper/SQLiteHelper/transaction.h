#pragma once
#include "database.h"

namespace SQLiteHelper
{
	//The default transaction behavior is DEFERRED.
	enum Behavior {
		DEFERRED,
		IMMEDIATE,
		EXCLUSIVE,
	};

	class Transaction {
	private:
		Database* ref_db;
		bool commited = false;
	public:
		explicit Transaction(Database* db);
		explicit Transaction(Database* db, Behavior behavior);
		Transaction(const Transaction&) = delete;				// Transaction is non-copyable
		Transaction& operator=(const Transaction&) = delete;	// Transaction is non-copyable
		~Transaction();
		void commit();
		void rollback();
	};
}