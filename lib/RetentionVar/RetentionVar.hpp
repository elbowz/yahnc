#pragma once

#include <functional>
#include <Arduino.h>

/*
// TODO
// * add OnChange to RetentionVar class...and rename OnUpdate => updateCondition/Guard
// * not declared inside RetentionVar to avoid use with template parameter (ie. RetentionVar<int>::ChangeValueReason::NO_CHANGE)
// * not used the scoped "enum class" to allow implicit casting
namespace ChangeValue {
    enum reason : uint8_t {
        NO_CHANGE = 0,
        INIT,
        GAP_TIME,
        GAP_RATE,
        GAP_ABS,
        UPDATE_HANDLER,
        FORCED
    };
}*/

enum class ChangeValueReason : uint8_t {
    NO_CHANGE = 0,
    INIT,
    GAP_TIME,
    GAP_RATE,
    GAP_ABS,
    UPDATE_HANDLER,
    FORCED
};

template<class T>
class RetentionVar {
public:
    typedef std::function<bool(T newValue, T currentValue, uint32_t updateTime, uint32_t gapTime, uint8_t gapRate, T gapAbs)> OnUpdateFunc;

private:
    virtual T absDiff(T a, T b) { return a > b ? a - b : b - a; }

protected:
    uint32_t mUpdateTime = 0;
    T mValue;
    T mLastValue;
    ChangeValueReason mUpdateReason = ChangeValueReason::NO_CHANGE;
    uint32_t mGapTime;
    uint8_t mGapRate;
    T mGapAbs;
    OnUpdateFunc mOnUpdateFunc;

    /**
    * Called on update request. Return true to allow the update, false otherwise.
    * This function can be set or override through inheritance way.
    * @param newValue
    * @param currentValue
    * @param updateTime
    * @param gapTime
    * @param gapRate
    * @param gapAbs
    * @return true for allow update
    */
    virtual bool onUpdate(T newValue, T currentValue, uint32_t updateTime, uint32_t gapTime, uint8_t gapRate, T gapAbs);

public:

    /**
    * A sort of variable that allow a value update only under certain condition
    * @param gapTime 0 to disable
    * @param gapRate 0 to disable
    * @param gapAbs 0 to disable
    * @param onUpdateFunc return true for allow
    */
    explicit RetentionVar(uint32_t gapTime, uint8_t gapRate = 0, T gapAbs = 0,
                          const RetentionVar::OnUpdateFunc &onUpdateFunc = [](T newValue, T currentValue, uint32_t updateTime, uint32_t gapTime, uint8_t gapRate, T gapAbs) { return false; });

    bool checkTime();

    bool update(T value);

    ChangeValueReason updateWithReason(T value);

    ChangeValueReason setValue(T value, ChangeValueReason reason = ChangeValueReason::FORCED, uint32_t now = millis());

    operator T() const { return mValue; }

    T value() const { return mValue; }

    uint32_t getGapTime() const { return mGapTime; }

    uint8_t getGapRate() const { return mGapRate; }

    T getGapAbs() const { return mGapAbs; }

    ChangeValueReason getUpdateReason() const { return mUpdateReason; }

    uint8_t getIntUpdateReason() const { return static_cast<uint8_t>(mUpdateReason); }

    void setOnUpdateFunc(const OnUpdateFunc &onUpdateFunc) { mOnUpdateFunc = onUpdateFunc; }
};

// Definition and implementation must be in the same file for template,
// otherwise throw "undefined reference to" on linker
// https://stackoverflow.com/a/1111470/2853788
// Possible ugly alternative, keep separated files and add all the type possibility:
/*template class RetentionVar<int>;
template class RetentionVar<long>;*/

template<class T>
RetentionVar<T>::RetentionVar(uint32_t gapTime, uint8_t gapRate, T gapAbs, const RetentionVar::OnUpdateFunc &onUpdateFunc)
        : mGapTime(gapTime), mGapRate(gapRate), mGapAbs(gapAbs), mOnUpdateFunc(onUpdateFunc) {
}

template<class T>
bool RetentionVar<T>::checkTime() {
    return (!mUpdateTime || millis() - mUpdateTime >= mGapTime);
}

template<class T>
bool RetentionVar<T>::update(T value) {
    return static_cast<bool>(updateWithReason(value));
}

template<class T>
ChangeValueReason RetentionVar<T>::updateWithReason(T value) {
    mLastValue = value;

    if (!mUpdateTime) {
        return setValue(value, ChangeValueReason::INIT);
    }

    T diff = (mGapRate || mGapAbs ? absDiff(mValue, value) : 0);

    if (diff) {

        if (mGapRate) {
            T gap = (mValue / 100) * mGapRate;

            if (diff >= gap) {
                return setValue(value, ChangeValueReason::GAP_RATE);
            }
        }

        if (mGapAbs && diff >= mGapAbs) {
            return setValue(value, ChangeValueReason::GAP_ABS);
        }
    }

    if (mGapTime && checkTime()) {
        return setValue(value, ChangeValueReason::GAP_TIME);
    }

    if (onUpdate(value, mValue, mUpdateTime, mGapTime, mGapRate, mGapAbs)) {
        return setValue(value, ChangeValueReason::UPDATE_HANDLER);
    }

    return ChangeValueReason::NO_CHANGE;
}

template<class T>
ChangeValueReason RetentionVar<T>::setValue(T value, ChangeValueReason reason, uint32_t now) {
    mUpdateTime = now;
    mUpdateReason = reason;
    mLastValue = mValue = value;

    return mUpdateReason;
}

template<class T>
bool RetentionVar<T>::onUpdate(T newValue, T currentValue, uint32_t updateTime, uint32_t gapTime, uint8_t gapRate, T gapAbs) {
    return mOnUpdateFunc(newValue, currentValue, updateTime, gapTime, gapRate, gapAbs);
}