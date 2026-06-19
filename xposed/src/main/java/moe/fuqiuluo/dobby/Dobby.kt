package moe.fuqiuluo.dobby

object Dobby {

    external fun setStatus(status: Boolean)

    external fun setStepMockStatus(status: Boolean, frequency: Int, baseCount: Long)

    external fun setStepCount(count: Long)

    external fun setStepFrequency(frequency: Int)
}