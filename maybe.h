#pragma once

template <typename T>
class Maybe{
	T *value;

	Maybe(void):value(NULL){}

public:
	Maybe(T v):value(new T(move(v))){}
	~Maybe(void){
		if(value)delete value;
	}

	T fromJust(void){
		return *value;
	}

	T& fromMaybe(T &def){
		if(value)return *value;
		else return def;
	}

	bool isJust(void){
		return (bool)value;
	}

	bool isNothing(void){
		return !value;
	}

	static Maybe<T> Nothing(void){
		return Maybe<T>();
	}
};
