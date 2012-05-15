from sqlalchemy import Column, Integer, String, Boolean, ForeignKey
from sqlalchemy.orm import relation
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm.exc import NoResultFound, MultipleResultsFound
import sauerbomber
from core.db import dbmanager
from core.events import eventHandler, triggerServerEvent, registerServerEventHandler, registerPolicyEventHandler
#from server.commands import commandHandler, UsageError, StateError, ArgumentValueError
from core.colors import red, green, orange
from core.ui import info, error, warning
# from ..players import player
from core.settings import PluginConfig
from server.users.ban import ban
from core.timers import addTimer
import re

config = PluginConfig('usermanager')
usertable = config.getOption('Config', 'users_tablename', 'usermanager_users')
nicktable = config.getOption('Config', 'linkednames_table', 'usermanager_nickaccounts')
blocked_names = config.getOption('Config', 'blocked_names', 'unnamed, admin')
del config

blocked_names = blocked_names.strip(' ').split(',')

Base = declarative_base()
session = dbmanager.session()

class User(Base):
	__tablename__ = usertable
	id = Column(Integer, primary_key=True)
	email = Column(String, index=True)
	password = Column(String, index=True)
	def __init__(self, email, password):
		self.email = email
		self.password = password

class NickAccount(Base):
	__tablename__ = nicktable
	id = Column(Integer, primary_key=True)
	nick = Column(String, index=True)
	user_id = Column(Integer, ForeignKey('usermanager_users.id'))
	user = relation(User, primaryjoin=user_id==User.id)
	def __init__(self, nick, user_id):
		self.nick = nick
		self.user_id = user_id

def loggedInAs(cn):
	return player(cn).user

def isLoggedIn(cn):
	try:
		return player(cn).logged_in
	except (AttributeError, ValueError):
		return False

def login(cn, user):
	if isLoggedIn(cn):
		raise StateError('You are already logged in')
	player(cn).user = user
	player(cn).logged_in = True
	triggerServerEvent('player_logged_in', (cn,))
	sauerbomber.message(info(green(sauerbomber.playerName(cn)) + ' is verified'))

def userAuth(email, password):
	try:
		user = dbmanager.query(User).filter(User.email==email).filter(User.password==password).one()
	except (NoResultFound, MultipleResultsFound):
		return False
	return user

def isValidEmail(email):
	if len(email) > 7:
		if re.match("^.+\\@(\\[?)[a-zA-Z0-9\\-\\.]+\\.([a-zA-Z]{2,3}|[0-9]{1,3})(\\]?)$", email) != None:
			return True
	return False

#@commandHandler('register')
def onRegisterCommand(cn, args):
	'''@description Register account with server
	   @usage email password
	   @public'''
	args = args.split(' ')
	if len(args) != 2:
		raise UsageError()
	try:
		dbmanager.query(User).filter(User.email==args[0]).one()
	except NoResultFound:
		if not isValidEmail(args[0]):
			raise ArgumentValueError('Invalid email address')
		user = User(args[0], args[1])
		session.add(user)
		session.commit()
		sauerbomber.playerMessage(cn, info('Account created'))
		return
	except MultipleResultsFound:
		pass
	raise StateError('An account with that email already exists')

#@commandHandler('login')
def onLoginCommand(cn, args):
	'''@description Login to server account
	   @usage email password
	   @public'''
	args = args.split(' ')
	if len(args) != 2:
		raise UsageError()
	user = userAuth(args[0], args[1])
	if user:
		login(cn, user)
	else:
		sauerbomber.playerMessage(cn, error('Invalid login.'))

#@commandHandler('linkname')
def onLinkName(cn, args):
	'''@description Link name to server account, and reserve name.
	   @usage
	   @public'''
	if args != '':
		raise UsageError()
	if not isLoggedIn(cn):
		raise StateError('You must be logged in to link a name to your account')
	if sauerbomber.playerName(cn) in blocked_names:
		raise StateError('You can not reserve this name')
	try:
		dbmanager.query(NickAccount).filter(NickAccount.nick==sauerbomber.playerName(cn)).one()
	except NoResultFound:
		user = loggedInAs(cn)
		nickacct = NickAccount(sauerbomber.playerName(cn), user.id)
		session.add(nickacct)
		session.commit()
		sauerbomber.playerMessage(cn, info('Your name is now linked to your account.'))
		sauerbomber.playerMessage(cn, info('You may now login with /setmaster password'))
		return
	except MultipleResultsFound:
		pass
	raise StateError('Your name is already linked to an account')

#@commandHandler('newuser')
def onNewuserCommand(cn, args):
	'''@description Register account with server
	   @usage email password
	   @public'''
	onRegisterCommand(cn, args)
	onLoginCommand(cn, args)
	onLinkName(cn, '')

#@commandHandler('changepass')
def onChangepass(cn, args):
	'''@description Link name to server account, and reserve name.
	   @usage
	   @public'''
	args = args.split(' ')
	if len(args) != 2:
		raise UsageError()
	if not isLoggedIn(cn):
		raise StateError('You must be logged in to change your password')
	try:
		dbmanager.query(User).filter(User.id==loggedInAs(cn).id).filter(User.password==args[0]).one()
	except NoResultFound:
		raise StateError('Incorrect password.')
	except MultipleResultsFound:
		pass
	else:
		dbmanager.query(User).filter(User.id==loggedInAs(cn).id).update({ 'password': args[1] })
		session.commit()
		return


@eventHandler('player_setmaster')
def onSetMaster(cn, givenhash):
	p = player(cn)
	adminhash = sauerbomber.hashPassword(cn, sauerbomber.adminPassword())
	try:
		na = dbmanager.query(NickAccount).filter(NickAccount.nick==p.name()).one()
	except NoResultFound:
		if givenhash != adminhash:
			p.message(error('Your name is not assigned to any accounts'))
	except MultipleResultsFound:
		p.message(error('Multiple names linked to this account.  Contact the system administrator.'))
	else:
		nickhash = sauerbomber.hashPassword(cn, na.user.password)
		if givenhash == nickhash:
			login(cn, na.user)
		else:
			if givenhash != adminhash:
				p.message(error('Invalid password'))

def warnNickReserved(cn, count, sessid):
	try:
		p = player(cn)
	except ValueError:
		return
	try:
		nickacct = p.warn_nickacct
		if nickacct.nick != sauerbomber.playerName(cn) or sessid != sauerbomber.playerSessionId(cn):
			p.warning_for_login = False
			return
	except (AttributeError, ValueError):
		p.warning_for_login = False
		return
	if isLoggedIn(cn):
		user = loggedInAs(cn)
		if nickacct.user_id != user.id:
			ban(cn, 0, 'Use of reserved name', -1)
			pass
		p.warning_for_login = False
		return
	if count > 4:
		ban(cn, 0, 'Use of reserved name', -1)
		p.warning_for_login = False
		return
	remaining = 25-(count*5)
	sauerbomber.playerMessage(cn, warning('Your name is reserved. You have ' + red('%i') + ' seconds to login or be kicked.') % remaining)
	addTimer(5000, warnNickReserved, (cn, count+1, sessid))

def nickReserver(nick):
	return dbmanager.query(NickAccount).filter(NickAccount.nick==nick).one()

@eventHandler('player_connect')
def onPlayerActive(cn):
	nick = sauerbomber.playerName(cn)
	p = player(cn)
	try:
		nickacct = nickReserver(sauerbomber.playerName(cn))
	except NoResultFound:
		p.warning_for_login = False
		return
	p = player(cn)
	p.warning_for_login = True
	p.warn_nickacct = nickacct
	warnNickReserved(cn, 0, sauerbomber.playerSessionId(cn))

@eventHandler('player_name_changed')
def onPlayerNameChanged(cn, old_name, new_name):
	# onPlayerActive(cn)
	pass

Base.metadata.create_all(dbmanager.engine)

