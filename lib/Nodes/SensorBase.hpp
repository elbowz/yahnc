#pragma once

#include <functional>
#include <Homie.hpp>

#include <RetentionVar.hpp>

template<class T>
class SensorBase : public RetentionVar<T> {
public:
    typedef std::function<T()> ReadMeasurementFunc;
    typedef std::function<void(T, ChangeValueReason)> SendMeasurementFunc;

protected:
    const char *mName;
    ReadMeasurementFunc mReadMeasurementFunc;
    SendMeasurementFunc mSendMeasurementFunc;
    // note: overkilled use of RetentionVar...better HomieInternals::Timer
    RetentionVar<T> mReadMeasurement;

    /**
     * Called to take/read the measurement and return the value
     * This function can be set or override through inheritance way.
     * @return read value
     */
    virtual T readMeasurement();

    /**
     * Called to send the measurement
     * This function can be set or override through inheritance way.
     * @return read value
     */
    virtual void sendMeasurement(T value, ChangeValueReason reason) const;

public:
    explicit SensorBase(const char *name, uint32_t readInterval = 300 * 1000UL,
                        uint8_t sendOnChangeRate = 2,
                        T sendOnChangeAbs = 0,
                        const ReadMeasurementFunc &readMeasurementFunc = nullptr,
                        const SendMeasurementFunc &sendMeasurementFunc = nullptr);

    virtual void loop();

    SensorBase<T> &setReadMeasurementFunc(const ReadMeasurementFunc &func) {
        mReadMeasurementFunc = func;
        return *this;
    }

    SensorBase<T> &setSendMeasurementFunc(const SendMeasurementFunc &func) {
        mSendMeasurementFunc = func;
        return *this;
    }

    const char *getName() const {
        return mName;
    }
};

// Definition and implementation must be in the same file for template,
// otherwise throw "undefined reference to" on linker
// https://stackoverflow.com/a/1111470/2853788

template<class T>
SensorBase<T>::SensorBase(const char *name, uint32_t readInterval,
                          uint8_t sendOnChangeRate,
                          T sendOnChangeAbs,
                          const SensorBase::ReadMeasurementFunc &readMeasurementFunc,
                          const SensorBase::SendMeasurementFunc &sendMeasurementFunc)
        : RetentionVar<T>(0, sendOnChangeRate, sendOnChangeAbs),
          mName(name),
          mReadMeasurementFunc(readMeasurementFunc),
          mSendMeasurementFunc(sendMeasurementFunc),
          mReadMeasurement(readInterval) {
}

template<class T>
void SensorBase<T>::loop() {

    if (mReadMeasurement.checkTime()) {

        T value = readMeasurement();

        /* Homie.getLogger() << F("Read sensor ") << getName()
                           << F(" each ") << mReadMeasurement.getGapTime()
                           << F(" ms: ") << value << endl;*/

        // Force a post-update
        mReadMeasurement.setValue(value, ChangeValueReason::GAP_TIME);

        // Why using "this->": https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
        ChangeValueReason reason = this->updateWithReason(value);
        if (static_cast<bool>(reason)) { sendMeasurement(value, reason); }
    }
}

template<class T>
T SensorBase<T>::readMeasurement() {
    if (!mReadMeasurementFunc) {
        HomieInternals::Helpers::abort(F("✖ ReadMeasurementFunc() is not set!"));
    }

    return mReadMeasurementFunc();
}

template<class T>
void SensorBase<T>::sendMeasurement(T value, ChangeValueReason reason) const {
    if (!mSendMeasurementFunc) {
        HomieInternals::Helpers::abort(F("✖ SendMeasurementFunc() is not set!"));
    }

    mSendMeasurementFunc(value, reason);
}
