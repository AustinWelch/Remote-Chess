import string
import random
import chess
import chess.engine
import firebase_admin
from firebase_admin import credentials, firestore
from os import error
from flask import Flask

try:
    cred = credentials.Certificate('./hey-frame-firestore-key.json')
    firebase_admin.initialize_app(cred)
    db = firestore.client()
except:
    pass

engine = chess.engine.SimpleEngine.popen_uci(".\stockfish_13_win_x64\stockfish_13_win_x64.exe")

app = Flask(__name__)


def get_piece_number(pos):
        return (ord(pos[0]) - 97) + ((ord(pos[1]) - 49) * 8)

def add_move_modifiers(board, move, is_move=False):
    pos1 = get_piece_number(move[:2])
    pos2 = get_piece_number(move[2:])

    if(len(move[2:]) == 3):
        if(is_move):
            promoted_piece = move[4:]
            move = move[:4] + 'P' + promoted_piece.upper()

        elif(move[4] == 'q'):
            move = move[:4] + 'P'
        
        else: 
            return 'PROMOTION_COPY'

    if(board.is_en_passant(chess.Move(pos1, pos2))):
        move = move + 'E'
    elif(board.is_capture(chess.Move(pos1, pos2))):
        move = move + 'A'
    if(board.is_kingside_castling(chess.Move(pos1, pos2))):
        move = move + 'K'
    if(board.is_queenside_castling(chess.Move(pos1, pos2))):
        move = move + 'Q'

    return move

def to_uci(piece_number):
    letter = piece_number % 8
    if (letter == 0):
        letter = 8

    return chr(int((letter % 8) + 97)) + chr(int(((piece_number / 8) + 49)))


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
            ,'name' : 'Player'
            ,'invites' : {}
            ,'incoming_pending_friends' : {}
            ,'outgoing_pending_friends' : {}
        })
        userInfo = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        return '<span style="white-space: pre-wrap">' + 'Friends:\n' + str(userInfo['friends']) +  '\nGames:\n' + str(userInfo['ongoing_game']) +'\n</span>'


@app.route('/user/<boardid>/setname/<new_name>')
def set_name(boardid, new_name):
    try:
        db.collection('chess').document('users').collection('users').document(boardid).update({ 'name':new_name })
        return '<span style="white-space: pre-wrap">Name successfully updated to' + new_name + '\n</span>'
    except:
        return '<span style="white-space: pre-wrap">Name could not be updated.</span>'


@app.route('/user/<boardid>/getname')
def get_name(boardid):
    try:
        name = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['name']
        return '<span style="white-space: pre-wrap">Name: ' + name + '</span>'
    except:
        return '<span style="white-space: pre-wrap">Name could not be retrieved.</span>'


@app.route('/user/<boardid>/getgame')
def get_game(boardid):
    try:
        ongoing_game = str(db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['ongoing_game'])
        if(ongoing_game == ''):
            return '<span style="white-space: pre-wrap">Not in a game.</span>'
        else:
            game = db.collection('chess').document('games').collection('games').document(ongoing_game).get().to_dict()
            print(boardid)
            if (boardid == game['player1_id']):
                player_color = 'WHITE'
            else:
                player_color = 'BLACK'
            return '<span style="white-space: pre-wrap">Game code: ' + ongoing_game + ' ' + player_color + '</span>'
    except:
        return '<span style="white-space: pre-wrap">User does not exist!</span>'


@app.route('/user/<boardid>/getfriends')
def get_friends(boardid):
    try:
        user_col = db.collection('chess').document('users').collection('users')
        user = user_col.document(boardid).get().to_dict()
        friends = user['friends']
        outgoing_reqs = user['outgoing_pending_friends']
        incoming_reqs = user['incoming_pending_friends']

        return_list = ''

        if(len(friends) == 0):
            return_list = 'No Friends'

        else:
            changed = False

            for key in friends:
                friend_name = friends[key]
                current_friend_name = user_col.document(key).get().to_dict()['name']
                if(friend_name != current_friend_name):
                    friends[key] = current_friend_name
                    changed = True
                return_list = return_list + key + ', ' + friends[key] + ', '

            if(changed):
                user_col.document(boardid).update({ 'friends':friends })

        return_list = return_list + ';'

        if(len(incoming_reqs) == 0):
            return_list = return_list + 'No Incoming Reqs'

        else:
            for key in incoming_reqs:
                return_list = return_list + key + ', ' + incoming_reqs[key] + ', '

        return_list = return_list + ';'

        if(len(outgoing_reqs) == 0):
             return_list = return_list + 'No Outgoing Reqs'

        else:
            for key in outgoing_reqs:
                return_list = return_list + key + ', ' + outgoing_reqs[key] + ', '
        
        return_list = return_list + ';'

        return '<span style="white-space: pre-wrap">' + 'Friends: ' + return_list +'</span>'
    
    except:
        return '<span style="white-space: pre-wrap">User does not exist!</span>'


@app.route('/user/<boardid>/newgame')
def create_new_game(boardid):
    def createId():
        id = ''.join(random.choice(string.digits) for _ in range(6))

        if(db.collection('chess').document('games').collection('games').document(id).get().to_dict()):
            return createId()
            
        return id

    board = chess.Board()
    id = createId()
    db.collection('chess').document('games').collection('games').document(id).set({
          'fen' : board.fen()
        , 'lastmove' : 'No previous move'
        , 'player1_id' : boardid
        , 'player2_id' : 0
        , 'players_joined' : 1
        , 'players_turn' : boardid
        , 'movecount' : 0
        , 'in_check' : 'N'
        , 'game_status' : 'O' #O -ongoing, R - opponent retired, W - winner decided
    })
    db.collection('chess').document('users').collection('users').document(boardid).update({
        'ongoing_game' : id
    })
    return '<span style="white-space: pre-wrap">' + 'New game created!\nId: ' + id + ' \n</span>'


@app.route('/user/<boardid>/newgamecpu')
def create_new_game_cpu(boardid):
    return_string = create_new_game(boardid)
    id = return_string.split(' ')[5]

    db.collection('chess').document('games').collection('games').document(id).update({
          'player2_id' : 'CPU'
        , 'players_joined' : 2
    })

    return '<span style="white-space: pre-wrap">' + 'New game vs CPU created!\nId: ' + id + '\n</span>'


@app.route('/user/<boardid>/newgamelocal')
def create_new_game_local(boardid):
    return_string = create_new_game(boardid)
    id = return_string.split(' ')[5]

    db.collection('chess').document('games').collection('games').document(id).update({
          'player2_id' : 'LOCAL_PLAY'
        , 'players_joined' : 2
    })

    return '<span style="white-space: pre-wrap">' + 'New local game created!\nId: ' + id + '\n</span>'


@app.route('/user/<boardid>/joingame/<game_code>')
def join_game(boardid, game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    
    if(not game):
        return '<span style="white-space: pre-wrap">' + 'Game with code ' + game_code + ' does not exist!</span>'

    if(game['players_joined'] < 2):
        db.collection('chess').document('users').collection('users').document(boardid).update({
            'ongoing_game' : game_code
        })
        game['player2_id'] = boardid
        game['players_joined'] = 2
        db.collection('chess').document('games').collection('games').document(game_code).set(game)
        return '<span style="white-space: pre-wrap">' + 'Successfully joined game: ' + game_code + '\n</span>'
    elif(game['player1_id'] == boardid or game['player2_id'] == boardid):
        return '<span style="white-space: pre-wrap">' + 'You have already joined this game!' '\n</span>'
    else:
        return '<span style="white-space: pre-wrap">' + 'Game is full!' + '\n</span>'


@app.route('/user/<boardid>/addfriend/<friendid>')
def add_friend(boardid, friendid):
    try:
        friend = db.collection('chess').document('users').collection('users').document(friendid).get().to_dict()
        friend_name = friend['name']

        user = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()
        friends = user['friends']

        if friendid in friends:
            return '<span style="white-space: pre-wrap">' + friend_name + ' already added as friend. </span>'

        user['outgoing_pending_friends'][friendid] = friend_name
        friend['incoming_pending_friends'][boardid] = user['name']

        db.collection('chess').document('users').collection('users').document(boardid).update({'outgoing_pending_friends' : user['outgoing_pending_friends']})
        db.collection('chess').document('users').collection('users').document(friendid).update({'incoming_pending_friends' : friend['incoming_pending_friends']})

        return '<span style="white-space: pre-wrap">Sent a friend request to ' + friend_name + '!</span>'

    except:
        return '<span style="white-space: pre-wrap">User with ID: ' + friendid + ' was not found.</span>'


@app.route('/user/<boardid>/cancelfriend/<friendid>')
def cancel_friend(boardid, friendid):
    try:
        friend = db.collection('chess').document('users').collection('users').document(friendid).get().to_dict()
        user = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()

        incoming_pending_friends = friend['incoming_pending_friends']
        outgoing_pending_friends = user['outgoing_pending_friends']

        if(len(incoming_pending_friends) == 0 or not boardid in incoming_pending_friends):
            return '<span style="white-space: pre-wrap">No friend request with user ' + friendid + '</span>'

        del incoming_pending_friends[boardid]
        del outgoing_pending_friends[friendid]

        db.collection('chess').document('users').collection('users').document(friendid).update({ 'incoming_pending_friends' :  incoming_pending_friends })
        db.collection('chess').document('users').collection('users').document(boardid).update({ 'outgoing_pending_friends' :  outgoing_pending_friends })
        
        return '<span style="white-space: pre-wrap">Canceled friend request to ' + friend['name'] + '</span>'

    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/acceptfriend/<sender_id>')
def accept_friend(boardid, sender_id):
    try:
        sender = db.collection('chess').document('users').collection('users').document(sender_id).get().to_dict()
        user = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()

        incoming_pending_friends = user['incoming_pending_friends']
        if(not sender_id in incoming_pending_friends):
            return '<span style="white-space: pre-wrap">No friend request from user ' + sender_id + '</span>'

        user['friends'][sender_id] = sender['name']
        sender['friends'][boardid] = user['name']

        del user['incoming_pending_friends'][sender_id]
        del sender['outgoing_pending_friends'][boardid]

        db.collection('chess').document('users').collection('users').document(boardid).update({
             'friends' : user['friends']
            ,'incoming_pending_friends' : user['incoming_pending_friends']
        })
        db.collection('chess').document('users').collection('users').document(sender_id).update({
             'friends' : sender['friends']
            ,'outgoing_pending_friends' : sender['outgoing_pending_friends']
        })

        return '<span style="white-space: pre-wrap">' + sender['name'] + ' added as a friend</span>'
    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/declinefriend/<sender_id>')
def decline_friend(boardid, sender_id):
    try:
        sender = db.collection('chess').document('users').collection('users').document(sender_id).get().to_dict()
        user = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()

        incoming_pending_friends = user['incoming_pending_friends']
        if(not sender_id in incoming_pending_friends):
            return '<span style="white-space: pre-wrap">No friend request from user ' + sender_id + '</span>'

        del user['incoming_pending_friends'][sender_id]
        del sender['outgoing_pending_friends'][boardid]

        db.collection('chess').document('users').collection('users').document(boardid).update({ 'incoming_pending_friends' : user['incoming_pending_friends'] })
        db.collection('chess').document('users').collection('users').document(sender_id).update({ 'outgoing_pending_friends' : sender['outgoing_pending_friends'] })

        return '<span style="white-space: pre-wrap">Declined ' + sender['name'] + '\'s friend request</span>'
    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/removefriend/<friendid>')
def remove_friend(boardid, friendid):
    friends = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['friends']
    if friendid in friends:
        del friends[friendid]
        db.collection('chess').document('users').collection('users').document(boardid).update({'friends' : friends})

        removedFriendFriends = db.collection('chess').document('users').collection('users').document(friendid).get().to_dict()['friends']
        del removedFriendFriends[boardid]
        db.collection('chess').document('users').collection('users').document(friendid).update({'friends' : removedFriendFriends})

        return '<span style="white-space: pre-wrap">Removed friend.</span>'

    return '<span style="white-space: pre-wrap">Friend was not found.\n</span>'


@app.route('/user/<boardid>/invites')
def get_invites(boardid):
    try:
        invites = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['invites']
        if(len(invites) == 0):
            return '<span style="white-space: pre-wrap">No invites pending</span>'

        parsedInvites = "Invites: "
        for key in invites:
            parsedInvites += key + ',' + invites[key][0] + ',' + invites[key][1] + ';'

        return parsedInvites

    except:
        return '<span style="white-space: pre-wrap">User with ID: '+ boardid + ' does not exist</span>'


@app.route('/user/<boardid>/sendinvite/<inviteeid>')
def send_invites(boardid, inviteeid):
    try:
        invitee = db.collection('chess').document('users').collection('users').document(inviteeid).get().to_dict()
        inviter = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()

        if (inviter['ongoing_game']):
            return '<span style="white-space: pre-wrap">You are already in a game</span>'

        if (invitee['ongoing_game']):
            return '<span style="white-space: pre-wrap">User ' + inviteeid + ' is already in a game</span>'
        
        create_new_game(boardid)

        inviter = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()

        invitee['invites'][boardid] = [inviter['name'], inviter['ongoing_game']]
        db.collection('chess').document('users').collection('users').document(inviteeid).update({ 'invites' :  invitee['invites'] })

        return '<span style="white-space: pre-wrap">Invite sent successfully. Game Code: ' + inviter['ongoing_game'] + '</span>'

    except:
        return '<span style="white-space: pre-wrap">User with ID: '+ inviteeid + ' does not exist</span>'


@app.route('/user/<boardid>/cancelinvite/<inviteeid>')
def cancel_invite(boardid, inviteeid):
    try:
        invitee = db.collection('chess').document('users').collection('users').document(inviteeid).get().to_dict()
        invites = invitee['invites']
        if(len(invites) == 0 or not boardid in invites):
            return '<span style="white-space: pre-wrap">No invite pending with user ' + inviteeid + '</span>'

        del invites[boardid]
        db.collection('chess').document('users').collection('users').document(inviteeid).update({ 'invites' :  invites })

        delete_game(db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['ongoing_game'])

        return '<span style="white-space: pre-wrap">Canceled invite with user ' + inviteeid + '</span>'

    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/acceptinvite/<inviterid>')
def accept_invite(boardid, inviterid):
    try:
        invites = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['invites']
        if(len(invites) == 0 or not inviterid in invites):
            return '<span style="white-space: pre-wrap">Invite does not exist</span>'

        join_game(boardid, invites[inviterid][1])

        del invites[inviterid]
        db.collection('chess').document('users').collection('users').document(boardid).update({ 'invites' :  invites })

        return '<span style="white-space: pre-wrap">Invite successfully accepted</span>'

    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/declineinvite/<inviterid>')
def decline_invite(boardid, inviterid):
    try:
        invites = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['invites']
        if(len(invites) == 0 or not inviterid in invites):
            return '<span style="white-space: pre-wrap">Invite does not exist</span>'

        delete_game(invites[inviterid][1])

        del invites[inviterid]
        db.collection('chess').document('users').collection('users').document(boardid).update({ 'invites' :  invites })

        return '<span style="white-space: pre-wrap">Invite declined</span>'

    except:
        return '<span style="white-space: pre-wrap">User does not exist</span>'


@app.route('/user/<boardid>/getlastinvite')
def get_last_invite(boardid):
    invites = db.collection('chess').document('users').collection('users').document(boardid).get().to_dict()['invites']
    if (len(invites) == 0):
        return '<span style="white-space: pre-wrap">No invite pending</span>'
    
    for key in invites:
        return '<span style="white-space: pre-wrap">Last Invite: ' + key + ';' + invites[key][0] + '!' + invites[key][1] + '</span>'


@app.route('/user/<boardid>/leavegame')
def leave_game(boardid):
    db.collection('chess').document('users').collection('users').document(boardid).update({ 'ongoing_game' : '' })
    return '<span style="white-space: pre-wrap">Left game successfully</span>'


@app.route('/game/<game_code>')
def get_code(game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    board = chess.Board(game['fen'])
    return '<span style="white-space: pre-wrap;font-family: &quot;Courier New&quot;;">' + 'Total Moves: ' + str(game['movecount']) + '\nPlayers in Game: P1: ' + str(game['player1_id']) + '   P2: ' + str(game['player2_id']) + '\nTurn: ' + str(game['players_turn']) + '\nLast Move: ' + str(game['lastmove']) + '\n' + board._repr_svg_() + '</span>'


@app.route('/game/<game_code>/gamestate')
def get_gamestate(game_code):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()   
    if (game['players_turn'] == game['player1_id'] ):
        player_number = 'P1:'
    else:
        player_number = 'P2:'


    return '<span style="white-space: pre-wrap">' + 'Turn: ' + player_number + str(game['players_turn']) + ', Last Move: ' + str(game['lastmove']) + ', Players: ' + 'P1:' + str(game['player1_id']) + ' P2:' + str(game['player2_id']) + ', Winner: ' + game['game_status'] + ', In Check: ' + game['in_check'] + '</span>'


@app.route('/game/<game_code>/delete')
def delete_game(game_code):
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
        return '<span style="white-space: pre-wrap">Game not found\n</span>'


@app.route('/game/<game_code>/turnready/<user_id>')
def is_turn_ready(game_code, user_id):
    print(user_id)
    try:
        game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
        if (game['player1_id'] != user_id and game['player2_id'] != user_id):
            return '<span style="white-space: pre-wrap">You are not in this game</span>'

        if (game['players_joined'] == 1):
            return '<span style="white-space: pre-wrap">Waiting for opponent to join</span>'

        if(game['players_turn'] == 'CPU'):
            board = chess.Board(game['fen'])
            stockfish.set_fen_position(game['fen'])
            if (not board.is_game_over()):
                result = stockfish.get_best_move_time(50)
                make_move(game_code, 'CPU', result)
                game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()

        status = game['game_status']
        if ('R' in status):
                return '<span style="white-space: pre-wrap">Opponent has resigned.</span>'

        if (game['players_turn'] == user_id):
            game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
            
            in_check = game['in_check']
            last_move = game['lastmove']
            if (len(last_move) == 0):
                last_move = 'No previous move'
                
            return '<span style="white-space: pre-wrap">Turn Ready - Last Move: ' + last_move + ', In Check: ' + in_check + ', Winner: ' + game['game_status'] + '</span>'
        else:
            return '<span style="white-space: pre-wrap">Turn Not Ready ' + 'Winner: '  + db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()['game_status'] + '</span>'
    except Exception as exc:
        print(exc)
        return '<span style="white-space: pre-wrap">Game does not exist</span>'


@app.route('/game/<game_code>/resign/<boardid>')
def resign(game_code, boardid):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()
    board = chess.Board(game['fen'])

    new_status = 'R:' + boardid + '!' + to_uci(board.king(board.turn)) + '|' + to_uci(board.king(not board.turn))
    db.collection('chess').document('games').collection('games').document(game_code).update({ 'game_status':new_status })

    if(boardid != 'LOCAL_PLAY'):
        db.collection('chess').document('users').collection('users').document(boardid).update({
                'ongoing_game' : ''
        })

    return '<span style="white-space: pre-wrap">Resigned from game ' + game_code + '</span>'


@app.route('/game/<game_code>/getlegalmoves')
def get_all_moves(game_code):

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
        all_legal_moves[i] = add_move_modifiers(board, all_legal_moves[i])

        if(all_legal_moves[i] != 'PROMOTION_COPY'):
            try:
                legal_moves[all_legal_moves[i][:2]].append(all_legal_moves[i][2:])
            except:
                legal_moves[all_legal_moves[i][:2]] = [get_piece_type(board, all_legal_moves[i][:2]), all_legal_moves[i][2:]]

    return '<span style="white-space: pre-wrap"> Legal moves:' + str(legal_moves) + '</span>'

    
@app.route('/game/<game_code>/undo')
def undo(game_code):   
    try:
        prev_state = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()['prev_state']
        db.collection('chess').document('games').collection('games').document(game_code).set(prev_state)
        return 'Undid.'
    except:
        return 'Cannot undo what is already undone.'


@app.route('/game/<game_code>/makemove/<userId>/<move>')
def make_move(game_code, userId, move):
    game = db.collection('chess').document('games').collection('games').document(game_code).get().to_dict()

    if('prev_state' in game):
        del game['prev_state']

    prev_state = dict(game)
     
    board = chess.Board(game['fen'])
    if(game['players_turn'] == userId):
        try:
            display_move = add_move_modifiers(board, move, True)
            board.push_uci(move)
            game['movecount'] += 1
            
            if (game['player1_id'] == userId):
                game['players_turn'] = game['player2_id']
            else:
                game['players_turn'] = game['player1_id']

            game['lastmove'] = display_move
            game['fen'] = board.fen()

            if(board.is_check()):
                game['in_check'] = 'Y!' + to_uci(board.king(board.turn))
            else:
                game['in_check'] = 'N'

            if(board.is_checkmate()):
                board = chess.Board(game['fen'])
                
                game['game_status'] = 'W:' + str(userId) + '!' + to_uci(board.king(board.outcome().winner)) + '|' + to_uci(board.king(board.turn))
            else:
                game['game_status'] = 'N'

            db.collection('chess').document('games').collection('games').document(game_code).set(game)
            db.collection('chess').document('games').collection('games').document(game_code).update({ 'prev_state' : prev_state })

            return '<span style="white-space: pre-wrap">Success! Move:' + str(game['movecount']) + '\n' + display_move + '\n' + str(board) + '</span>'

        except Exception as exc:
            print(exc)
            return '<span style="font-family: &quot;Courier New&quot;;white-space: pre-wrap">Move not valid.</span>'
    else:
        return '<span style="white-space: pre-wrap">It is not your turn.</span>'

if __name__ == '__main__':
    app.run()
