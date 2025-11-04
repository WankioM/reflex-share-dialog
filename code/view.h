#pragma once

#include "instance.h"




//
//declarations

REFLEX_NS(ShareBox)

class View : public Bootstrap::View
{
public:

	static TRef <View> Create(Instance & instance);



protected:

	using Bootstrap::View::View;

};

REFLEX_END
