import string
import random
import chess
import firebase_admin
from firebase_admin import credentials, firestore
from os import error
from flask import Flask

try:
    cred = credentials.Certificate('hey-frame-firestore-key.json')
    firebase_admin.initialize_app(cred)
    db = firestore.client()
except:
    pass

app = Flask(__name__)


@app.route('/')
def home_page():
    return 'Server for Remote Chess :)'


@app.route('/ping')
def ping():
    return '<span style="white-space: pre-wrap">pong\n</span>'


@app.route('/user/<boardid>')
def user_page(boardid):
    try:
        userInfo = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + str(userInfo['friends']) +  '\nGames:\n' + str(userInfo['ongoing_game']) +'\n</span>'
    except:
        db.collection('chess').document('users').collection('users').document(boardid).set({
             'friends' : {}
            ,'ongoing_game' : ''
            ,'name':'unknown'
        })
        userInfo = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + str(userInfo['friends']) +  '\nGames:\n' + str(userInfo['ongoing_game']) +'\n</span>'


@app.route('/user/<boardid>/setname/<new_name>')
def set_name(boardid, new_name):
    try:
        db.collection('chess').document('users').collection('users').document(boardid).update({ 'name':new_name })
        return '<span style="white-space: pre-wrap">Name successfully updated to' + new_name + '\n</span>'
    except:
        return '<span style="white-space: pre-wrap">Name could not be updated.\n</span>'


@app.route('/user/<boardid>/getgame')
def get_game(boardid):
    try:
        ongoing_game = str(db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['ongoing_game'])
        if(ongoing_game == ''):
            return '<span style="white-space: pre-wrap">Not in a game.\n</span>'
        else:
            return '<span style="white-space: pre-wrap">Game code: ' + ongoing_game + '</span>'
    except:
        return '<span style="white-space: pre-wrap">User does not exist!\n</span>'


@app.route('/user/<boardid>/getfriends')
def get_friends(boardid):
    try:
        user_col = db.collection('chess').document('users').collection('users')
        friends = user_col.document(boardid).get().to_dict()['friends']

        if(len(friends) == 0):
            return '<span style="white-space: pre-wrap">You currently have no friends added.\n</span>'

        return_list = ''
        changed = False

        for key in friends:
            friend_name = friends[key]
            current_friend_name = user_col.document(key).get().to_dict()['name']
            if(friend_name != current_friend_name):
                friends[key] = current_friend_name
                changed = True
            return_list = return_list + friends[key] +'\n'

        if(changed):
            user_col.document(boardid).update({ 'friends':friends })

        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + return_list +'\n</span>'
    
    except:
        return '<span style="white-space: pre-wrap">User does not exist!\n</span>'


@app.route('/user/<boardid>/newgame')
def create_new_game(boardid):
    def createId():
        id = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(7))

        if(db.collection('chess').document('games').collection('games').document(id).get().to_dict()):
            return createId()
            
        return id

    board = chess.Board()
    id = createId()
    db.collection('chess').document('games').collection('games').document(id).set({
          'chessboard' : str(board)
        , 'fen' : board.fen()
        , 'lastmove' : ''
        , 'player1_id' : int(boardid)
        , 'player2_id' : 0
        , 'players_joined' : 1
        , 'players_turn' : int(boardid)
        , 'movecount' : 0
    })
    db.collection('chess').document('users').collection('users').document(boardid).update({
        'ongoing_game' : id
    })
    return '<span style="white-space: pre-wrap">' + 'New game created!\nId: ' + id + '\n</span>'


@app.route('/user/<boardid>/joingame/<game_code>')
def join_game(boardid, game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    if(game['players_joined'] < 2):
        db.collection('chess').document('users').collection('users').document(boardid).update({
            'ongoing_game' : game_code
        })
        game['player2_id'] = int(boardid)
        game['players_joined'] = 2
        db.collection('chess').document('games').collection('games').document(game_code).set(game)
        return '<span style="white-space: pre-wrap">' + 'Successfully joined game: ' + str(game_code) + '\n</span>'
    elif(game['player1_id'] == boardid or game['player2_id'] == boardid):
        return '<span style="white-space: pre-wrap">' + 'You have already joined this game!' '\n</span>'
    else:
        return '<span style="white-space: pre-wrap">' + 'Game is full!' + '\n</span>'


@app.route('/user/<boardid>/addfriend/<friendid>')
def add_friend(boardid, friendid):
    try:
        friend_name = db.collection('chess').document('users').collection('users').document(friendid).get().to_dict()['name']
        friends = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['friends']
        if(friends[friendid]):
            return '<span style="white-space: pre-wrap">User with ID: ' + friendid + ' already added as friend. \n</span>'

        friends[friendid] = friend_name
        db.collection('chess').document('users').collection('users').document(boardid).update({'friends' : friends})
        return '<span style="white-space: pre-wrap">User with ID: ' + friendid + ' added as friend ' + friend_name + '!\n</span>'

    except:
        return '<span style="white-space: pre-wrap">User with ID: ' + friendid + ' was not found.\n</span>'


@app.route('/game/<game_code>')
def get_code(game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    return '<span style="white-space: pre-wrap">' + 'Total Moves: ' + str(game['movecount']) + '\nPlayers in Game: P1: ' + str(game['player1_id']) + '   P2: ' + str(game['player2_id']) + '\nTurn: ' + str(game['players_turn']) + '\nLast Move: ' + str(game['lastmove']) +'\nBoard:\n' + str(game['chessboard']) + '</span>'


@app.route('/game/<game_code>/delete')
def delete_board(game_code):
    try:
        game_ref = db.collection('chess').document('games').collection('games').document(game_code)
        game = game_ref.get().to_dict()
        player1 = game['player1_id']
        player2 = game['player2_id']
        db.collection('chess').document('users').collection('users').document(str(player1)).update({
            'ongoing_game' : ''
        })
        db.collection('chess').document('users').collection('users').document(str(player2)).update({
            'ongoing_game' : ''
        })
        game_ref.delete()
        return '<span style="white-space: pre-wrap">' + 'Deleted Game ' + game_code + '\n</span>'
    except:
        return '<span style="white-space: pre-wrap">Game not found!\n</span>'


@app.route('/game/<game_code>/getboard')
def get_board(game_code):
    return '<span style="white-space: pre-wrap">' + db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()['chessboard'] + '</span>'


@app.route('/game/<game_code>/turnready/<int:user_id>')
def is_turn_ready(game_code, user_id):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    if (game['players_joined'] == 1):
        return '<span style="white-space: pre-wrap">Waiting for opponent to join.</span>'
    elif (game['players_turn'] == user_id):
        return '<span style="white-space: pre-wrap">Turn Ready.</span>'
    else:
        return '<span style="white-space: pre-wrap">Turn Not Ready.</span>'


@app.route('/game/<game_code>/getlegalmoves')
def get_all_moves(game_code):

    def get_piece_number(pos):
        return (ord(pos[0]) - 97) + ((ord(pos[1]) - 49) * 8)

    def get_piece_type(board, pos):
        piece_number = get_piece_number(pos)
        piece_type = str(board.piece_at(piece_number)).upper()
                    
        if piece_type == 'P':
            piece_type = 'Pawn'
        elif piece_type == 'R':
            piece_type = 'Rook'
        elif piece_type == 'N':
            piece_type = 'Knight'
        elif piece_type == 'B':
            piece_type = 'Bishop'
        elif piece_type == 'Q':
            piece_type = 'Queen'
        elif piece_type == 'K':
            piece_type = 'King'

        return piece_type

    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()

    board = chess.Board(game['fen'])
    
    all_legal_moves = list(board.legal_moves)
    for i in range(len(all_legal_moves)):
        all_legal_moves[i] = str(all_legal_moves[i])

    legal_moves = {}

    for i in range(len(all_legal_moves)):
        if(board.is_en_passant(chess.Move(get_piece_number(all_legal_moves[i][:2]), get_piece_number(all_legal_moves[i][2:])))):
            all_legal_moves[i] = all_legal_moves[i] + 'E'
        elif(board.is_capture(chess.Move(get_piece_number(all_legal_moves[i][:2]), get_piece_number(all_legal_moves[i][2:])))):
            all_legal_moves[i] = all_legal_moves[i] + 'A'
        if(board.is_kingside_castling(chess.Move(get_piece_number(all_legal_moves[i][:2]), get_piece_number(all_legal_moves[i][2:])))):
            all_legal_moves[i] = all_legal_moves[i] + 'K'
        if(board.is_queenside_castling(chess.Move(get_piece_number(all_legal_moves[i][:2]), get_piece_number(all_legal_moves[i][2:])))):
            all_legal_moves[i] = all_legal_moves[i] + 'Q'

        try:
            legal_moves[all_legal_moves[i][:2]].append(all_legal_moves[i][2:])
        except:
            legal_moves[all_legal_moves[i][:2]] = [get_piece_type(board, all_legal_moves[i][:2]), all_legal_moves[i][2:]]

    return '<span style="white-space: pre-wrap"> Legal moves:' + str(legal_moves) + '</span>'

    
@app.route('/game/<game_code>/makemove/<int:userId>/<move>')
def existing_game(game_code, userId, move):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
 
    board = chess.Board(game['fen'])
    if(game['players_turn'] == userId):
        try:
            board.push_uci(move)
            game['movecount'] += 1
            
            if (game['player1_id'] == userId):
                game['players_turn'] = game['player2_id']
            else:
                game['players_turn'] = game['player1_id']

            game['lastmove'] = move
            game['chessboard'] = str(board)
            game['fen'] = board.fen()

            db.collection('chess').document('games').collection('games').document(game_code).set(game)

            return '<span style="white-space: pre-wrap">Success! Move:' + str(game['movecount']) + '\n' + move + '\n' + str(board) + '</span>'

        except Exception as exc:
            print(exc)
            return '<span style="white-space: pre-wrap">Move not valid.</span>'
    else:
        return '<span style="white-space: pre-wrap">It is not your turn.</span>'


if __name__ == '__main__':
    app.run()
