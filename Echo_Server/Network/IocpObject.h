#pragma once
#include "IoEvent.h"

class IocpObject : public std::enable_shared_from_this<IocpObject> {
public:
	IocpObject() = default;
	~IocpObject() = default;
public:
	virtual void OnExecute(IoEvent* iocpEvnet) abstract;
};