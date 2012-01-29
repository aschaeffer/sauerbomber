from core.events import eventHandler, triggerServerEvent, execLater
from core.timers import addTimer
from core.net import ipLongToString, ipStringToLong
from core.ui import insufficientPermissions, error
from server.users.privilege import isUserMaster, isUserAdmin, isUserAtLeastMaster
import sauerbomber
import logging
import math

def isMaster(cn):
	if sauerbomber.playerPrivilege(cn) == 1:
		return True
	try:
		return isUserMaster(player(cn).user.id)
	except AttributeError, ValueError:
		return False

def isAtLeastMaster(cn):
	if sauerbomber.playerPrivilege(cn) > 0:
		return True
	try:
		return isUserAtLeastMaster(player(cn).user.id)
	except AttributeError, ValueError:
		return False

def isAdmin(cn):
	if sauerbomber.playerPrivilege(cn) == 2:
		return True
	try:
		return isUserAdmin(player(cn).user.id)
	except AttributeError, ValueError:
		return False

class masterRequired(object):
	def __init__(self, func):
		self.func = func
		self.__doc__ = func.__doc__
		self.__name__ = func.__name__
	def __call__(self, *args):
		if not isAtLeastMaster(args[0]):
			insufficientPermissions(args[0])
		else:
			self.func(*args)

class adminRequired(object):
	def __init__(self, func):
		self.func = func
		self.__doc__ = func.__doc__
		self.__name__ = func.__name__
	def __call__(self, *args):
		if not isAdmin(args[0]):
			insufficientPermissions(args[0])
		else:
			self.func(*args)

class Player:
	'''Represents a client on the server'''
	def __init__(self, cn):
		self.cn = cn
		self.gamevars = {}
	def newGame(self):
		'''Reset game variables'''
		self.gamevars.clear()
	def sessionId(self):
		'''Session ID of client'''
		return sauerbomber.playerSessionId(self.cn)
	def name(self):
		'''Name of client'''
		return sauerbomber.playerName(self.cn)
	def ipLong(self):
		'''Ip of client as long'''
		return sauerbomber.playerIpLong(self.cn)
	def ipString(self):
		'''Ip of client as decimal octet string'''
		return ipLongToString(self.ipLong())
	def privilege(self):
		'''Integer privilege of client'''
		return sauerbomber.playerPrivilege(self.cn)
	def frags(self):
		'''Frags by client in current game'''
		return sauerbomber.playerFrags(self.cn)
	def teamkills(self):
		'''Team kills by client in current game'''
		return sauerbomber.playerTeamkills(self.cn)
	def deaths(self):
		'''Deaths by client in current game'''
		return sauerbomber.playerDeaths(self.cn)
	def ping(self):
		'''Last reported ping of client'''
		return sauerbomber.playerPing(self.cn)
	def team(self):
		'''Name of team client belongs to'''
		return sauerbomber.playerTeam(self.cn)
	def isSpectator(self):
		'''Is client a spectator'''
		return sauerbomber.playerIsSpectator(self.cn)
	def isMaster(self):
		return isMaster(self.cn)
	def isAtLeastMaster(self):
		return isAtLeastMaster(self.cn)
	def isAdmin(self):
		return isAdmin(self.cn)
	def message(self, msg):
		'''Send message to client'''
		sauerbomber.playerMessage(self.cn, msg)
	def kick(self):
		'''Disconnect client from server'''
		execLater(sauerbomber.playerKick, (self.cn,))
	def spectate(self):
		'''Make client spectator'''
		sauerbomber.spectate(self.cn)
	def unspectate(self):
		'''Make client not a spectator'''
		sauerbomber.unspectate(self.cn)
	def setTeam(self, team):
		'''Set team client belongs to'''
		sauerbomber.setTeam(self.cn, team)
	def suicide(self):
		'''Force client to commit suicide'''
		sauerbomber.suicide(self.cn)
	def shots(self):
		'''Shots the player has fired'''
		return sauerbomber.playerShots(self.cn)
	def hits(self):
		'''Number of hits the player has dealt'''
		return sauerbomber.playerHits(self.cn)
	def accuracy(self):
		'''Accuracy of player'''
		shots = self.shots()
		accuracy = 0
		if shots != 0:
			accuracy = self.hits() / float(shots)
			accuracy = math.floor(accuracy * 100)
		return accuracy
	def kpd(self):
		'''Kills Per Death'''
		deaths = self.deaths()
		frags = self.frags()
		if deaths == 0:
			kpd = frags
		else:
			kpd = frags / float(deaths)
			kpd *= float(100)
			kpd = math.floor(kpd)
			kpd = kpd / 100
		return kpd
	def score(self):
		'''Flags the player has scored'''
		return sauerbomber.playerScore(self.cn)

players = {}

@eventHandler('map_changed')
def onMapChanged(mapname, mapmode):
	for player in players.values():
		player.newGame();

def all():
	'''Get list of all clients'''
	return players.values()

def cnsToPlayers(cns):
	'''Turn list of cn's into list of Player's'''
	ps = []
	for cn in cns:
		ps.append(player(cn))
	return ps

def clientCount():
	'''Number of clients connected to server'''
	return len(sauerbomber.clients())

def spectatorCount():
	'''Number of spectators in server'''
	return len(sauerbomber.spectators())

def playerCount():
	'''Number of players in game'''
	return len(sauerbomber.players())

def spectators():
	'''Get list of spectators as Player instances'''
	return cnsToPlayers(sauerbomber.spectators())

def activePlayers():
	'''Get list of players as Player instances'''
	return cnsToPlayers(sauerbomber.players())

def player(cn):
	'''Return player instance of cn'''
	try:
		return players[cn]
	except KeyError:
		raise ValueError('Player does not exist')

def playerByName(name):
	'''Return player instance of player with name'''
	for p in all():
		if p.name() == name:
			return p
	raise ValueError('No player by specified name')

def playerByIpLong(ip):
	'''Return Player instance with matching long (int) ip'''
	for p in all():
		if ip == p.ipLong():
			return p
	raise ValueError('No player found matching ip')

def playerByIpString(ip):
	'''Return Player instance with matching string ip'''
	return playerByIpLong(ipStringToLong(ip))

@eventHandler('player_disconnect_post')
@eventHandler('game_bot_removed')
def playerDisconnect(cn):
	try:
		del players[cn]
	except KeyError:
		logging.error('Player disconnected but does not have player class instance!')

def triggerConnectDelayed(cn):
	try:
		player(cn)
	except ValueError:
		return
	else:
		triggerServerEvent('player_connect_delayed', (cn,))

def currentMaster():
	for p in all():
		if p.privilege() == 1:
			return p
	return None

def currentAdmin():
	for p in all():
		if p.privilege() == 2:
			return p
	return None

def addPlayerForCn(cn):
	try:
		del players[cn]
	except KeyError:
		pass
	players[cn] = Player(cn)

@eventHandler('player_connect_pre')
def onPlayerConnect(cn):
	addPlayerForCn(cn)
	addTimer(1000, triggerConnectDelayed, (cn,))

@eventHandler('game_bot_added')
def onBotConnect(cn):
	addPlayerForCn(cn)

@eventHandler('restart_complete')
def reload():
	for cn in sauerbomber.clients():
		playerConnect(cn)

@eventHandler('player_auth_succeed')
def onAuthSuccess(cn, name):
	if currentAdmin() != None:
		sauerbomber.playerMessage(cn, error('Admin is present'))
		return
	sauerbomber.setMaster(cn)

