#pragma once

template <typename T,typename U>
class Either{
	T *leftv;
	U *rightv;

	Either(void):leftv(NULL),rightv(NULL){}

public:
	~Either(void){
		if(leftv)delete leftv;
		if(rightv)delete rightv;
	}

	T fromLeft(void) const {
		return *leftv;
	}

	U fromRight(void) const {
		return *rightv;
	}

	bool isLeft(void) const {
		return (bool)leftv;
	}

	bool isRight(void) const {
		return (bool)rightv;
	}

	static Either<T,U> Left(T v){
		Either<T,U> e;
		e.leftv=new T(move(v));
		return e;
	}
	static Either<T,U> Right(U v){
		Either<T,U> e;
		e.rightv=new U(move(v));
		return e;
	}
};
