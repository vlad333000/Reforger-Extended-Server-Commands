class V30_ShutdownAfterMissionScheduler {
	protected static ref V30_ShutdownAfterMissionScheduler s_Instance = null;
	
	protected bool m_bScheduled = false;
	
	protected void V30_ShutdownAfterMissionScheduler() {
		return;
	};
	
	static V30_ShutdownAfterMissionScheduler GetInstance() {
		if (!s_Instance) {
			s_Instance = new V30_ShutdownAfterMissionScheduler();
		};

		return s_Instance;
	};
	
	void Schedule() {
		auto restart = V30_RestartAfterMissionScheduler.GetInstance();
		if (restart.IsScheduled()) {
			restart.Unschedule();
		};
		m_bScheduled = true;
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetOnGameEnd().Insert(OnGameEnd);
	};
	
	void Unschedule() {
		m_bScheduled = false;
		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetOnGameEnd().Remove(OnGameEnd);
	};
	
	bool IsScheduled() {
		return m_bScheduled;
	};
	
	protected void OnGameEnd() {
		GetGame().GetBackendApi().GetDSSession().RequestShutdown();
	};
};

class V30_ShutdownAfterMissionCommand : ScrServerCommand {
	protected bool m_bExecOnServer = false;
	
	override string GetKeyword() {
		return "shutdownaftermission";
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
	
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId) {
		if (argv.Count() < 2) {
			m_bExecOnServer = false;
			return ScrServerCmdResult("Invalid argument(s), type `#shutdownaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else if (argv[1] == "help") {
			m_bExecOnServer = false;
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else if (argv[1] != "1" && argv[1] != "0" && argv[1] != "check") {
			m_bExecOnServer = false;
			return ScrServerCmdResult("Invalid 1st argument, type `#shutdownaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else {
			m_bExecOnServer = true;
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
		};
	};
	
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId) {
		return HandleCommand(argv);
	};
	
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv) {
		return HandleCommand(argv);
	};
	
	override ref ScrServerCmdResult OnUpdate() {
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);	
	};
	
	protected ScrServerCmdResult HandleCommand(array<string> argv) {
		if (RplSession.Mode() != RplMode.Dedicated) {
			return ScrServerCmdResult("Command is supported only on Dedicated Servers", EServerCmdResultType.ERR);
		}
		else if (argv.Count() < 2) {
			return ScrServerCmdResult("Invalid argument(s), type `#shutdownaftermission help`", EServerCmdResultType.PARAMETERS);
		}
		else if (argv[1] == "1") {
			if (V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("Server shutdown after mission is already scheduled", EServerCmdResultType.BUSY);
			}
			else {
				V30_ShutdownAfterMissionScheduler.GetInstance().Schedule();
				return ScrServerCmdResult("Server will be shutdown after mission", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "0") {
			if (!V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("Server shutdown after mission is not scheduled yet", EServerCmdResultType.BUSY);
			}
			else {
				V30_ShutdownAfterMissionScheduler.GetInstance().Unschedule();
				return ScrServerCmdResult("Server will not be shutdown after mission", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "check") {
			if (V30_ShutdownAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("1 (server shutdown after mission is scheduled)", EServerCmdResultType.OK);
			}
			else if (V30_RestartAfterMissionScheduler.GetInstance().IsScheduled()) {
				return ScrServerCmdResult("0 (server shutdown after mission is not scheduled, server restart scheduled)", EServerCmdResultType.OK);
			}
			else {
				return ScrServerCmdResult("0 (server shutdown after mission is not scheduled)", EServerCmdResultType.OK);
			};
		}
		else if (argv[1] == "help") {
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else {
			return ScrServerCmdResult("Invalid 1st argument, possible values: 1 - schedule, 0 - unschedule, <empty> - return current state", EServerCmdResultType.PARAMETERS);
		};
	};
	
	string GetHelp() {
		return "Description: schedule the server shutdown at mission end\nUsage: #shutdownaftermission [1 | 0 | check | help]\nOption(s): 1 - schedule shutdown, 0 - unschedule shutdown, check - return current state, help - show this message";
	};
};