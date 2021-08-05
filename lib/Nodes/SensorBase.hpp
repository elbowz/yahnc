#pragma once

#include <functional>
#include <Homie.hpp>

#include <RetentionVar.hpp>


/**
 * TODO:
 * * nullptr also for default value of onLightChange ?
 * * rename readMeasurement/sendMeasurement to read/send
 * * move readMeasurement/sendMeasurement to public
 * * remove SensorInterface and use a Template Specialization of RetentionVar for bool ?
 * * add topic, dataType, format, unit (used for advertise and setProperty)...maybe extend PropertyInterface ?
 */

template<class T>
class SensorInterface {
public:
    typedef std::function<T()> ReadMeasurementFunc;
    typedef std::function<void(T)> SendMeasurementFunc;
    typedef std::function<bool(T)> OnChangeFunc;

protected:
    const char *mName;
    ReadMeasurementFunc mReadMeasurementFunc;
    SendMeasurementFunc mSendMeasurementFunc;
    OnChangeFunc mOnChangeFunc;

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
    virtual void sendMeasurement(T value) const;

    /**
     * Called on measurement change
     * This function can be set or override through inheritance way.
     * @return true to allow the sendMeasurement invocation
     */
    virtual bool onChange(T value) const;

public:
    explicit SensorInterface(const char *name,
                             const ReadMeasurementFunc &readMeasurementFunc = nullptr,
                             const SendMeasurementFunc &sendMeasurementFunc = nullptr,
                             const OnChangeFunc &onChangeFunc = [](bool value) { return true; });

    explicit SensorInterface(const char *name,
                             const OnChangeFunc &onChangeFunc = [](bool value) { return true; });

    virtual void loop() = 0;

    SensorInterface<T> &setReadMeasurementFunc(const ReadMeasurementFunc &func) {
        mReadMeasurementFunc = func;
        return *this;
    }

    SensorInterface<T> &setSendMeasurementFunc(const SendMeasurementFunc &func) {
        mSendMeasurementFunc = func;
        return *this;
    }

    SensorInterface<T> &setOnChangeFunc(const OnChangeFunc &func) {
        mOnChangeFunc = func;
        return *this;
    }

    const char *getName() const {
        return mName;
    }
};

template<class T>
SensorInterface<T>::SensorInterface(const char *name,
                                    const SensorInterface::ReadMeasurementFunc &readMeasurementFunc,
                                    const SensorInterface::SendMeasurementFunc &sendMeasurementFunc,
                                    const OnChangeFunc &onChangeFunc)
        : mName(name),
          mReadMeasurementFunc(readMeasurementFunc),
          mSendMeasurementFunc(sendMeasurementFunc),
          mOnChangeFunc(onChangeFunc) {}

template<class T>
SensorInterface<T>::SensorInterface(const char *name, const SensorInterface::OnChangeFunc &onChangeFunc)
        : SensorInterface(name, nullptr, nullptr, onChangeFunc) {}

template<class T>
T SensorInterface<T>::readMeasurement() {
    if (!mReadMeasurementFunc) {
        HomieInternals::Helpers::abort(F("✖ ReadMeasurementFunc() is not set!"));
    }

    return mReadMeasurementFunc();
}

template<class T>
void SensorInterface<T>::sendMeasurement(T value) const {
    if (!mSendMeasurementFunc) {
        HomieInternals::Helpers::abort(F("✖ SendMeasurementFunc() is not set!"));
    }

    if (onChange(value)) mSendMeasurementFunc(value);
}

template<class T>
bool SensorInterface<T>::onChange(T value) const {
    return mOnChangeFunc(value);
}


template<class T>
class SensorBase : public SensorInterface<T>, public RetentionVar<T> {
protected:
    // note: overkilled use of RetentionVar...better HomieInternals::Timer
    RetentionVar<T> mReadMeasurement;

public:
    explicit SensorBase(const char *name,
                        uint32_t readInterval = 300 * 1000UL,
                        uint8_t sendOnChangeRate = 2,
                        T sendOnChangeAbs = 0,
                        const typename SensorInterface<T>::ReadMeasurementFunc &readMeasurementFunc = nullptr,
                        const typename SensorInterface<T>::SendMeasurementFunc &sendMeasurementFunc = nullptr,
                        const typename SensorInterface<T>::OnChangeFunc &onChangeFunc = [](bool value) { return true; });

    virtual void loop();
};

// Definition and implementation must be in the same file for template,
// otherwise throw "undefined reference to" on linker
// https://stackoverflow.com/a/1111470/2853788

template<class T>
SensorBase<T>::SensorBase(const char *name,
                          uint32_t readInterval,
                          uint8_t sendOnChangeRate,
                          T sendOnChangeAbs,
                          const typename SensorInterface<T>::ReadMeasurementFunc &readMeasurementFunc,
                          const typename SensorInterface<T>::SendMeasurementFunc &sendMeasurementFunc,
                          const typename SensorInterface<T>::OnChangeFunc &onChangeFunc)
        :  SensorInterface<T>(name, readMeasurementFunc, sendMeasurementFunc, onChangeFunc),
           RetentionVar<T>(0, sendOnChangeRate, sendOnChangeAbs),
           mReadMeasurement(readInterval) {
}

template<class T>
void SensorBase<T>::loop() {

    if (mReadMeasurement.checkTime()) {

        T value = this->readMeasurement();

        /* Homie.getLogger() << F("Read sensor ") << getName()
                           << F(" each ") << mReadMeasurement.getGapTime()
                           << F(" ms: ") << value << endl;*/

        // Force a post-update
        mReadMeasurement.setValue(value, ChangeValueReason::GAP_TIME);

        // Why using "this->": https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
        ChangeValueReason reason = this->updateWithReason(value);

        if (static_cast<bool>(reason)) {
            // TODO: move on change here
            this->sendMeasurement(value);
        }
    }
}
