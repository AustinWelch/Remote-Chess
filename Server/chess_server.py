from os import error
from flask import Flask
import chess
import firebase_admin
from firebase_admin import credentials, firestore

try:
    cred = credentials.Certificate('hey-frame-firestore-key.json')
    firebase_admin.initialize_app(cred)
    db = firestore.client()
except:
    xyz = 0 #Do nothing

app = Flask(__name__)


@app.route('/')
def home_page():
    return 'Server for Online Chess :)'

@app.route('/user/<boardid>')
def user_page(boardid):
    try:
        userInfo = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + str(userInfo['friends']) +  '\nGames:\n' + str(userInfo['ongoing_games']) +'\n</span>'
    except:
        db.collection('chess').document('users').collection('users').document(boardid).set({
             'friends' : {}
            ,'ongoing_games' : []
        })
        userInfo = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + str(userInfo['friends']) +  '\nGames:\n' + str(userInfo['ongoing_games']) +'\n</span>'

@app.route('/user/<boardid>/newgame')
def create_new_game(boardid):
    board = chess.Board()
    res = db.collection('chess').document('games').collection('games').add({
          'chessboard' : str(board)
        , 'fen' : board.fen()
        , 'lastmove' : ""
        , 'player1_id' : int(boardid)
        , 'player2_id' : 0
        , 'players_joined' : 1
        , 'whose_turn' : int(boardid)
        , 'movecount' : 0
    })
    db.collection('chess').document('users').collection('users').document(boardid).update({
        'ongoing_games' : firestore.ArrayUnion([str(res[1].id)])
    })
    return '<span style="white-space: pre-wrap">' + 'New game created!\nId: ' + str(res[1].id) + '\n</span>'

@app.route('/user/<boardid>/joingame/<game_code>')
def join_game(boardid, game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    if(game['players_joined'] < 2):
        db.collection('chess').document('users').collection('users').document(boardid).update({
            'ongoing_games' : firestore.ArrayUnion([game_code])
        })
        game['player2_id'] = int(boardid)
        game['players_joined'] = 2
        db.collection('chess').document('games').collection('games').document(game_code).set(game)
        return '<span style="white-space: pre-wrap">' + 'Successfully joined game: ' + str(game_code) + '\n</span>'
    elif(game['player1_id'] == boardid or game['player2_id'] == boardid):
        return '<span style="white-space: pre-wrap">' + 'You have already joined this game!' '\n</span>'
    else:
        return '<span style="white-space: pre-wrap">' + 'Game is full!' + '\n</span>'

@app.route('/user/<boardid>/addfriend/<friendid>/<alias>')
def add_friend(boardid, friendid, alias):
    try:
        db.collection('chess').document('users').collection('users').document(friendid).get()
        friends = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['friends']
        friends[friendid] = alias
        db.collection('chess').document('users').collection('users').document(boardid).update({'friends' : friends})
        return '<span style="white-space: pre-wrap">User with ID: ' + friendid + ' added as friend ' + alias + '\n</span>'

    except:
        return '<span style="white-space: pre-wrap">User with ID: ' + friendid + 'was not found.\n</span>'

@app.route('/game/<game_code>')
def get_game(game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    return '<span style="white-space: pre-wrap">' + 'Total Moves: ' + str(game['movecount']) + '\nPlayers in Game: P1: ' + str(game['player1_id']) + '   P2: ' + str(game['player2_id']) + '\nTurn: ' + str(game['whose_turn']) + '\nLast Move: ' + str(game['lastmove']) +'\nBoard:\n' + str(game['chessboard']) + '</span>'

@app.route('/game/<game_code>/delete')
def delete_board(game_code):
    game_ref = db.collection('chess').document('games').collection('games').document(game_code)
    game = game_ref.get().to_dict()
    player1 = game['player1_id']
    player2 = game['player2_id']
    db.collection('chess').document('users').collection('users').document(str(player1)).update({
        'ongoing_games' : firestore.ArrayRemove([game_code])
    })
    db.collection('chess').document('users').collection('users').document(str(player2)).update({
        'ongoing_games' : firestore.ArrayRemove([game_code])
    })
    game_ref.delete()
    return '<span style="white-space: pre-wrap">' + 'Deleted Game ' + game_code + '\n</span>'

@app.route('/game/<game_code>/getboard')
def get_board(game_code):
    return '<span style="white-space: pre-wrap">' + db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()['chessboard'] + '</span>'
    
@app.route('/game/<game_code>/makemove/<int:userId>/<move>')
def existing_game(game_code, userId, move):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    board = chess.Board(game['fen'])
    # if(game['whose_turn'] == userId):
    try:
        board.push_uci(move)
        game['movecount'] += 1
        
        if (game['player1_id'] == userId):
            game['whose_turn'] = game['player2_id']
        else:
            game['whose_turn'] = game['player1_id']

        game['lastmove'] = move
        game['chessboard'] = str(board)
        game['fen'] = board.fen()

        db.collection('chess').document('games').collection('games').document(game_code).set(game)

        return '<span style="white-space: pre-wrap">Success! Move:' + str(game['movecount']) + '\n' + move + '\n' + str(board) + '</span>'

    except Exception as exc:
        print(exc)
        return '<span style="white-space: pre-wrap">Move not valid.</span>'
    # else:
    #     return '<span style="white-space: pre-wrap">It is not your turn.</span>'


if __name__ == '__main__':
    app.run()
