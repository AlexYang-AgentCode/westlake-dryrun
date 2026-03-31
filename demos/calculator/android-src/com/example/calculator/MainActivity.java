package com.example.calculator;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
    private TextView display;
    private String current = "";
    private String operator = "";
    private double operand1 = 0;
    private boolean newNumber = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        display = (TextView) findViewById(R.id.display);

        int[] numIds = {R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3,
                        R.id.btn4, R.id.btn5, R.id.btn6, R.id.btn7,
                        R.id.btn8, R.id.btn9};
        for (int id : numIds) {
            findViewById(id).setOnClickListener(v -> onNumber(((Button)v).getText().toString()));
        }

        findViewById(R.id.btnAdd).setOnClickListener(v -> onOperator("+"));
        findViewById(R.id.btnSub).setOnClickListener(v -> onOperator("-"));
        findViewById(R.id.btnMul).setOnClickListener(v -> onOperator("x"));
        findViewById(R.id.btnDiv).setOnClickListener(v -> onOperator("/"));
        findViewById(R.id.btnEq).setOnClickListener(v -> onEquals());
        findViewById(R.id.btnC).setOnClickListener(v -> onClear());
    }

    private void onNumber(String n) {
        if (newNumber) { current = n; newNumber = false; }
        else { current += n; }
        display.setText(current);
    }

    private void onOperator(String op) {
        if (!current.isEmpty()) operand1 = Double.parseDouble(current);
        operator = op;
        newNumber = true;
    }

    private void onEquals() {
        if (current.isEmpty() || operator.isEmpty()) return;
        double operand2 = Double.parseDouble(current);
        double result = 0;
        switch (operator) {
            case "+": result = operand1 + operand2; break;
            case "-": result = operand1 - operand2; break;
            case "x": result = operand1 * operand2; break;
            case "/": result = operand2 != 0 ? operand1 / operand2 : 0; break;
        }
        if (result == (long) result) {
            display.setText(String.valueOf((long) result));
        } else {
            display.setText(String.valueOf(result));
        }
        current = display.getText().toString();
        operator = "";
        newNumber = true;
    }

    private void onClear() {
        current = "";
        operator = "";
        operand1 = 0;
        newNumber = true;
        display.setText("0");
    }
}
