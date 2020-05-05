package com.jcphilips.robotcontroller

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.widget.Button
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.firebase.database.ktx.database
import com.google.firebase.ktx.Firebase
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    // Write a message to the database
    private val database = Firebase.database
    private val robotMotor = database.getReference("wheels")
    private val armVerticalServo = database.getReference("verticalServo")
    private val armHorizontalServo = database.getReference("baseServo")
    private val gripperServo = database.getReference("gripperServo")
    private val debugTag = "MainActivity"
    interface OnTouchListener

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val robotMoveUp = findViewById<FloatingActionButton>(R.id.robotMoveUpActionButton)
        val robotMoveDown = findViewById<FloatingActionButton>(R.id.robotMoveDownActionButton)
        val gripperMoveUp = findViewById<FloatingActionButton>(R.id.gripperMoveUpActionButton)
        val gripperMoveDown = findViewById<FloatingActionButton>(R.id.gripperMoveDownActionButton)
        val gripperMoveLeft = findViewById<FloatingActionButton>(R.id.gripperMoveLeftActionButton)
        val gripperMoveRight = findViewById<FloatingActionButton>(R.id.gripperMoveRightActionButton)
        val closeGripper = findViewById<Button>(R.id.closeGripperButton)
        val openGripper = findViewById<Button>(R.id.openGripperButton)

        robotMoveUp.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> robotMotor.setValue(1)
                    MotionEvent.ACTION_UP -> robotMotor.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        robotMoveDown.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> robotMotor.setValue(2)
                    MotionEvent.ACTION_UP -> robotMotor.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        gripperMoveUp.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> armVerticalServo.setValue(1)
                    MotionEvent.ACTION_UP -> armVerticalServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        gripperMoveDown.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> armVerticalServo.setValue(2)
                    MotionEvent.ACTION_UP -> armVerticalServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        gripperMoveLeft.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> armHorizontalServo.setValue(1)
                    MotionEvent.ACTION_UP -> armHorizontalServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        gripperMoveRight.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> armHorizontalServo.setValue(2)
                    MotionEvent.ACTION_UP -> armHorizontalServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        closeGripper.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> gripperServo.setValue(2)
                    MotionEvent.ACTION_UP -> gripperServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
        openGripper.setOnTouchListener(object : View.OnTouchListener {
            override fun onTouch(v: View?, event: MotionEvent?): Boolean {
                when (event?.action) {
                    MotionEvent.ACTION_DOWN -> gripperServo.setValue(1)
                    MotionEvent.ACTION_UP -> gripperServo.setValue(0)
                }
                return v?.onTouchEvent(event) ?: true
            }
        })
    }
}
