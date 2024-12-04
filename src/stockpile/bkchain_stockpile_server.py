# Python script representing a blockchain
# Allows actions such as adding a new transaction, mining a block and checking the stock pile

from flask import Flask, request, jsonify
import hashlib
import json
import time
from typing import Dict, List, Any

# Initialize Flask application
app = Flask(__name__)

class BlockchainStockPile:
    def __init__(self):
        # Initialize the blockchain with a genesis block
        self.chain = []
        self.current_transactions = []
        self.create_block(previous_hash='1', proof=100)
    
    # Create a new block in the blockchain
    def create_block(self, proof: int, previous_hash: str = None) -> Dict:
        block = {
            'index': len(self.chain) + 1,  # Incremental block number
            'timestamp': time.time(),  # Current timestamp
            'transactions': self.current_transactions,  # Transactions in this block
            'proof': proof,  # Proof of work value
            'previous_hash': previous_hash or self.hash(self.chain[-1])  # Hash linking to previous block
        }
        
        # Reset current transactions after block creation
        self.current_transactions = []
        self.chain.append(block)
        return block
    
    # Create a new stock transaction
    def new_transaction(self, item: str, quantity: int, location: str) -> int:
        transaction = {
            'item': item,  # Type of stock item
            'quantity': quantity,  # Number of items
            'location': location,  # Storage location
            'timestamp': time.time()  # Transaction timestamp
        }
        
        self.current_transactions.append(transaction)
        return len(self.chain) + 1  # Return future block index
    
    # Create a SHA-256 hash of a block
    @staticmethod
    def hash(block: Dict) -> str:
        block_string = json.dumps(block, sort_keys=True).encode()
        return hashlib.sha256(block_string).hexdigest()
    
    # Retrieve stock information from the blockchain
    def get_stock_info(self, item: str = None) -> List[Dict]:
        results = []
        for block in self.chain:
            for transaction in block['transactions']:
                # Return all transactions or filter by item
                if item is None or transaction['item'] == item:
                    results.append(transaction)
        return results

# Global blockchain instance
blockchain = BlockchainStockPile()

# Endpoint to add a new stock transaction.
@app.route('/transaction', methods=['POST'])
def add_transaction():
    data = request.get_json()
    required = ['item', 'quantity', 'location'] # Obtain 'item', 'quantity', and 'location'
    
    # Validate required fields are present
    if not all(k in data for k in required):
        return jsonify({'error': 'Missing required fields'}), 400
    
    # Add transaction to blockchain
    index = blockchain.new_transaction(
        data['item'], 
        data['quantity'], 
        data['location']
    )
    
    return jsonify({
        'message': f'Transaction will be added to Block {index}',
        'block_index': index
    }), 201

# Endpoint to retrieve stock information
@app.route('/stock_info', methods=['GET'])
def get_stock_info():
    item = request.args.get('item')
    stock_info = blockchain.get_stock_info(item)
    return jsonify(stock_info)

# Endpoint to mine a new block in the blockchain using a simple PoW
@app.route('/mine', methods=['GET'])
def mine_block():
    last_block = blockchain.chain[-1]
    proof = len(blockchain.chain) + 1
    
    blockchain.create_block(proof)
    
    return jsonify({
        'message': 'New Block Forged',
        'index': len(blockchain.chain)
    })

if __name__ == '__main__':
    # Run Flask server on port 8000
    app.run(debug=False, port=8000)