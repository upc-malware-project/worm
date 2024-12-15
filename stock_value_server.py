# Python script for providing the (hardcoded) pseudo-stock value of Microsoft and the percentage of change 
# since the previous measurement
import random
from flask import Flask, Response

app = Flask(__name__)

# Initial stock value
stock_value = 1000
# Previous stock value
previous_stock_value = stock_value

@app.route('/stock/reset', methods=['GET'])
def reset_stock():
    global stock_value
    stock_value = 1000
    return Response("", mimetype='text/plain')

# Function for providing the current stock value of microsoft and percentage of change 
# since the previous measurement
@app.route('/stock/microsoft', methods=['GET'])
def get_microsoft_stock():
    global stock_value, previous_stock_value
    
    # Store current value as previous before updating
    previous_stock_value = stock_value
    
    # Randomly update stock value
    change = random.randint(-100, 100)
    stock_value += change
    
    # Calculate percentage of change
    percentage_change = ((stock_value - previous_stock_value) / previous_stock_value) * 100
    
    # Return stock value and percentage of change in plaintext
    response_text = f"MSFT,{stock_value},{percentage_change:.2f}"
    return Response(response_text, mimetype='text/plain')

# Main function
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
