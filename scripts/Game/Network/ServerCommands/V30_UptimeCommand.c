class V30_UptimeCommand : ScrServerCommand {
	protected static const int s_fTimeDelay = 60;
	
	protected static int s_fTime = -60;
	
	protected bool m_bExecOnServer = false;
	
	override string GetKeyword() {
		return "uptime";
	};
	
	override bool IsServerSide() {
		return m_bExecOnServer;
	};
	
	override int RequiredRCONPermission() {
		return ERCONPermissions.PERMISSIONS_NONE;
	};
	
	override int RequiredChatPermission() {
		return EPlayerRole.NONE;
	};
	
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId) {
		auto time = System.GetUnixTime();
		if (argv.Count() > 1 && argv[1] == "help") {
			m_bExecOnServer = false;
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else if (s_fTime + s_fTimeDelay > time && !GetGame().GetPlayerController().HasRole(EPlayerRole.ADMINISTRATOR)) {
			m_bExecOnServer = false;
			return ScrServerCmdResult(string.Format("You can't request server uptime more than once every %1 second(s) (%2 second(s) left)", s_fTimeDelay, s_fTime + s_fTimeDelay - time), EServerCmdResultType.BUSY);
		}
		else {
			m_bExecOnServer = true;
			s_fTime = time;
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
		};
	};
	
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId) {
		return HandleCommand(argv);
	};
	
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv) {
		if (argv.Count() > 1 && argv[1] == "help") {
			return ScrServerCmdResult(GetHelp(), EServerCmdResultType.OK);
		}
		else {
			return HandleCommand(argv);
		};
	};
	
	override ref ScrServerCmdResult OnUpdate() {
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);	
	};
	
	protected ScrServerCmdResult HandleCommand(array<string> argv) {
		if (RplSession.Mode() != RplMode.Dedicated) {
			return ScrServerCmdResult("Command is supported only on Dedicated Servers", EServerCmdResultType.ERR);
		}
		else if (argv.Count() < 2) {
			int t = Math.Floor(GetGame().GetBackendApi().GetDSSession().GetUpTime());
			string s = string.ToString(t % 60); t /= 60; if (s.Length() == 1) s = "0" + s;
			string m = string.ToString(t % 60); t /= 60; if (m.Length() == 1) m = "0" + m;
			string h = string.ToString(t % 24); t /= 24; if (h.Length() == 1) h = "0" + h;
			int d = t;
			return ScrServerCmdResult(string.Format("%1d %2h %3m %4s", d, h, m, s), EServerCmdResultType.OK);	
		}
		else {
			return ScrServerCmdResult("Invalid arguments", EServerCmdResultType.PARAMETERS);	
		};
	};
	
	string GetHelp() {
		return "Description: return uptime of server in format 'Dd HHh MMm SSs'\nUsage: #uptime [help]\nOption(s): help - show this message";
	};
};