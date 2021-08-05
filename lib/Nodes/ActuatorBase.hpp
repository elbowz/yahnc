#pragma once

#include <Homie.hpp>


template<class T>
class ActuatorBase {
public:
    typedef std::function<T()> GetStateFunc;
    typedef std::function<void(T)> SendStateFunc;
    typedef std::function<void(T)> SetHwStateFunc;
    typedef std::function<bool(T)> OnSetFunc;

protected:
    const char *mName;
    OnSetFunc mOnSetFunc;
    GetStateFunc mGetStateFunc;
    SetHwStateFunc mSetHwStateFunc;
    SendStateFunc mSendStateFunc;

    /**
     * Called on state is set/change
     * This function can be set or override through inheritance way.
     * @return true to allow the send invocation
     */
    virtual bool onSet(T value);

public:
    explicit ActuatorBase(const char *name,
                          const OnSetFunc &onSetFunc = [](bool value) { return true; },
                          const GetStateFunc &getStateFunc = nullptr,
                          const SetHwStateFunc &setHwStateFunc = nullptr,
                          const SendStateFunc &sendStateFunc = nullptr);

    virtual void setup() {};

    virtual void onReadyToOperate() {};

    virtual void loop() {};

    const char *getName() const {
        return mName;
    }

    /**
    * Called to get current state and return the value
    * This function can be set or override through inheritance way.
    * @return current state
    */
    virtual T getState();

    /**
     * Called to set state (hw and send)
     * @param value to set
     */
    virtual void setState(T value);

    /**
     * Called to set hardware state
     * This function can be set or override through inheritance way.
     * @param value to set
     */
    virtual void setHwState(T value) const;

    /**
     * Called to send state
     * This function can be set or override through inheritance way.
     * @param value to send
     */
    virtual void sendState(T value) const;

    ActuatorBase<T> &setGetStateFunc(const GetStateFunc &func) {
        mGetStateFunc = func;
        return *this;
    }

    ActuatorBase<T> &setSetHwStateFunc(const SetHwStateFunc &func) {
        mSetHwStateFunc = func;
        return *this;
    }

    ActuatorBase<T> &setSendStateFunc(const SendStateFunc &func) {
        mSendStateFunc = func;
        return *this;
    }

    ActuatorBase<T> &setOnSetFunc(const OnSetFunc &func) {
        mOnSetFunc = func;
        return *this;
    }
};

template<class T>
ActuatorBase<T>::ActuatorBase(const char *name,
                              const ActuatorBase::OnSetFunc &onSetFunc,
                              const ActuatorBase::GetStateFunc &getStateFunc,
                              const ActuatorBase::SetHwStateFunc &setHwStateFunc,
                              const ActuatorBase::SendStateFunc &sendStateFunc)
        : mName(name),
          mOnSetFunc(onSetFunc),
          mGetStateFunc(getStateFunc),
          mSetHwStateFunc(setHwStateFunc),
          mSendStateFunc(sendStateFunc) {}

template<class T>
T ActuatorBase<T>::getState() {
    if (!mGetStateFunc) {
        HomieInternals::Helpers::abort(F("✖ GetStateFunc() is not set!"));
    }

    return mGetStateFunc();
}

template<class T>
void ActuatorBase<T>::setState(T value) {

    if (onSet(value)) {
        setHwState(value);

        if (Homie.isConnected()) {
            sendState(value);
        }
    }
}

template<class T>
void ActuatorBase<T>::setHwState(T value) const {
    if (!mSetHwStateFunc) {
        HomieInternals::Helpers::abort(F("✖ SetHwStateFunc() is not set!"));
    }

    mSetHwStateFunc(value);
}

template<class T>
void ActuatorBase<T>::sendState(T value) const {
    if (!mSendStateFunc) {
        HomieInternals::Helpers::abort(F("✖ SendStateFunc() is not set!"));
    }

    mSendStateFunc(value);
}

template<class T>
bool ActuatorBase<T>::onSet(T value) {
    return mOnSetFunc(value);
}

