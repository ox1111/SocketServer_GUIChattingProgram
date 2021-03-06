#pragma once
#include "BaseQuery.h"
#include "BaseDB.h"

class ChatUserData;

class ChatQuery :
	public BaseQuery
{
public:
	virtual bool InitializeQuery() override;
	virtual void ReleaseQuery() override;
	
	/*유저가 DB에 저장되어있는지 확인합니다.
	TRUE : 유저가 존재합니다
	FALSE : 유저가 존재하지 않습니다*/
	bool CheckUserDataInDB(const std::string & strInId);
	bool FindUserDataInDB(const std::string& strInId, TYPE_ROW& row);
	bool CheckDuplicationUserName(const std::string& strInId);
	bool InsertUserDataInDB(const std::string& strInId, const std::string& strInPassword, const std::string& strInName);
};

