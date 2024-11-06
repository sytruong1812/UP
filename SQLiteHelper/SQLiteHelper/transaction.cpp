#include "transaction.h"
#include "database.h"

namespace SQLiteHelper
{
	Transaction::Transaction(Database* db) : ref_db(db)
	{
		ref_db->execute("BEGIN TRANSACTION");
	}

	Transaction::Transaction(Database* db, Behavior behavior) : ref_db(db)
	{
		switch (behavior) {
		case Behavior::DEFERRED:
			ref_db->execute("BEGIN DEFERRED");
			break;
		case Behavior::IMMEDIATE:
			ref_db->execute("BEGIN IMMEDIATE");
			break;
		case Behavior::EXCLUSIVE:
			ref_db->execute("BEGIN EXCLUSIVE");
			break;
		default:
			break;
		}
	}

	Transaction::~Transaction()
	{
		rollback();
	}

	void Transaction::commit()
	{
		if (commited == false) {
			if (ref_db->execute("COMMIT TRANSACTION")) {
				commited = true;
			}
		}
	}

	void Transaction::rollback()
	{
		if (commited == false) {
			ref_db->execute("ROLLBACK TRANSACTION");
		}
	}
}