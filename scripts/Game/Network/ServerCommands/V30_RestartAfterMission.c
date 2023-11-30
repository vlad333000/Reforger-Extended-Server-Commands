class V30_RestartAfterMissionScheduler {
	protected static ref V30_RestartAfterMissionScheduler s_Instance = null;
	
	protected bool m_bScheduled = false;
	
	protected void V30_RestartAfterMissionScheduler() {
		return;
	};
	
	static V30_RestartAfterMissionScheduler GetInstance() {
		if (!s_Instance) {
			s_Instance = new V30_RestartAfterMissionScheduler();
		};

		return s_Instance;
	};
	
	void Schedule() {
		if (V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
			return;
		};
		m_bScheduled = true;
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetOnGameModeEnd().Insert(OnGameModeEnd);
	};
	
	void Unschedule() {
		m_bScheduled = false;
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetOnGameModeEnd().Remove(OnGameModeEnd);
	};
	
	bool IsScheduled() {
		return m_bScheduled;
	};
	
	protected void OnGameModeEnd(SCR_GameModeEndData endData) {
		GetGame().GetBackendApi().GetDSSession().RequestRestart();
	};
};

class V30_RestartAfterMissionCommand : ScrServerCommand {
	protected bool m_bExecOnServer = false;
	
	override string GetKeyword() {
		return "restartaftermission";
	};
	
	override bool IsServerSide() {
		return m_bExecOnServer;
	};
	
	override int RequiredRCONPermission() {
		return ERCONPermissions.PERMISSIONS_ADMIN;
	};
	
	override int RequiredChatPermission() {
		return EPlayerRole.ADMINISTRATOR;
	};
	
	protected ScrServerCmdResult HandleCommand(array<string> argv) {
		if (RplSession.Mode() != RplMode.Dedicated) {
			return ScrServerCmdResult("Command is supported only on Dedicated Servers", EServerCmdResultType.ERR);
		}
		else if (argv.Count() < 2) {
			return ScrServerCmdResult("Invalid argument(s), type `#restartaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else if (argv[1] == "1") {
			if (V30_RestartAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("Server restart after mission is already scheduled", EServerCmdResultType.BUSY);
			}
			else if (V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("Can't shedule the server restart because shutdown already scheduled, unschedule shutdown first", EServerCmdResultType.ERR);
			}
			else {
				V30_RestartAfterMissionScheduler.GetInstance().Schedule();
				return ScrServerCmdResult("Server will be restart after mission", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "0") {
			if (!V30_RestartAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("Server restart after mission is not scheduled yet", EServerCmdResultType.BUSY);
			}
			else {
				V30_RestartAfterMissionScheduler.GetInstance().Unschedule();
				return ScrServerCmdResult("Server will not be restart after mission", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "check") {
			if (V30_RestartAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("1 (server restart after mission is scheduled)", EServerCmdResultType.OK);
			}
			else if (V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("0 (server restart after mission is not scheduled, server shutdown scheduled)", EServerCmdResultType.OK);
			}
			else {
				return ScrServerCmdResult("0 (server restart after mission is not scheduled)", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "help") {
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else {
			return ScrServerCmdResult("Invalid 1st argument use `#restartaftermission help`", EServerCmdResultType.PARAMETERS);
		};
	};
	
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId) {
		return HandleCommand(argv);
	};
	
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId) {
		if (argv.Count() < 2) {
			m_bExecOnServer = false;
			return ScrServerCmdResult("Invalid argument(s), use `#restartaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else if (argv[1] == "help") {
			m_bExecOnServer = false;
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else if (argv[1] != "1" && argv[1] != "0" && argv[1] != "check") {
			m_bExecOnServer = false;
			return ScrServerCmdResult("Invalid 1st argument, use `#restartaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else {
			m_bExecOnServer = true;
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
		};
	};
	
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv) {
		return HandleCommand(argv);
	};
	
	override ref ScrServerCmdResult OnUpdate() {
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.ERR);	
	};
	
	string GetHelp() {
		return "Description: schedule the server restart at mission end\nUsage: #restartaftermission [1 | 0 | check | help]\nOption(s): 1 - schedule restart, 0 - or unschedule restart, check - return current state, help - show this message";
	};
};