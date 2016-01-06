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

	T fromJust(void) const {
		return *value;
	}

	T fromMaybe(T &def) const {
		if(value)return *value;
		else return def;
	}

	bool isJust(void) const{
		return (bool)value;
	}

	bool isNothing(void) const{
		return !value;
	}

	static Maybe<T> Nothing(void){
		return Maybe<T>();
	}
};
