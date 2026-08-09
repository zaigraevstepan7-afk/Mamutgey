# Among Us 2026.6.5 (7045) arm64-v8a offsets
# Source: APKPure XAPK https://d.apkpure.net/b/XAPK/com.innersloth.spacemafia?version=latest
# Unity 2022.3.62f3 / metadata v31

## PlayerControl
public class PlayerControl : InnerNetObject // TypeDefIndex: 1473
### Fields
- 0x35: public byte PlayerId
- 0x38: public string FriendCode
- 0x40: public string Puid
- 0x48: public float MaxReportDistance
- 0x4C: public bool moveable
- 0x50: public CosmeticsLayer cosmetics
- 0x58: public bool ForceKillTimerContinue
- 0x5C: private PlayerOutfitType <CurrentOutfitType>k__BackingField
- 0x60: public bool inVent
- 0x61: public bool walkingToVent
- 0x62: public bool petting
- 0x63: public bool inMovingPlat
- 0x64: public bool onLadder
- 0x65: public bool protectedByGuardianThisRound
- 0x66: public bool shapeshifting
- 0x67: public bool waitingForShapeshiftResponse
- 0x68: public bool isKilling
- 0x6C: private float invisibilityAlpha
- 0x0: public static PlayerControl LocalPlayer
- 0x70: private NetworkedPlayerInfo CachedPlayerData
- 0x78: private int protectedByGuardianId
- 0x7C: private float flashlightAngle
- 0x80: private int shapeshiftTargetPlayerId
- 0x84: private bool shouldAppearInvisible
- 0x85: public bool isTrackingPlayer
- 0x88: public PlayerControl trackedPlayer
- 0x90: public int trackedPlayerColorID
- 0x98: public AudioSource FootSteps
- 0xA0: public AudioClip KillSfx
- 0xA8: public KillAnimation[] KillAnimations
- 0xB0: private float killTimer
- 0xB4: public int RemainingEmergencies
- 0xB8: public LightSource LightPrefab
- 0xC0: private LightSource lightSource
- 0xC8: public Collider2D Collider
- 0xD0: public PlayerPhysics MyPhysics
- 0xD8: public CustomNetworkTransform NetTransform
- 0xE0: private Collider2D clickKillCollider
- 0xE8: public Vector3 defaultCosmeticsScale
- 0xF8: public List<PlayerTask> myTasks
- 0x100: public List<RoleEffectAnimation> currentRoleAnimations
- 0x108: public GameObject TargetFlashlight
- 0x110: public bool isDummy
- 0x111: public bool notRealPlayer
- 0x118: private readonly Logger logger
- 0x120: private readonly List<IPlayerVisibleItem> visibilityItems
- 0x128: private Collider2D[] hitBuffer
- 0x130: private IUsable closest
- 0x138: private bool isNew
- 0x139: private bool hasBeenSerialized
- 0x140: private Rigidbody2D rigidbody2D
- 0x8: public static List<PlayerControl> AllPlayerControls
- 0x148: private Dictionary<Collider2D, IUsable[]> cache
- 0x150: private List<IUsable> itemsInRange
- 0x158: private List<IUsable> newItemsInRange
- 0x160: private byte scannerCount
- 0x161: private bool roleAssigned
- 0x164: private int LastStartCounter
### Methods (first 40)
- Offset 0x21B0EB0 / RVA 0x21B4EB0: public bool get_CanMove() { }
- Offset 0x21B11D0 / RVA 0x21B51D0: public bool get_IsKillTimerEnabled() { }
- Offset 0x21B13A8 / RVA 0x21B53A8: public NetworkedPlayerInfo.PlayerOutfit get_CurrentOutfit() { }
- Offset 0x21B14B4 / RVA 0x21B54B4: public PlayerOutfitType get_CurrentOutfitType() { }
- Offset 0x21B14BC / RVA 0x21B54BC: private void set_CurrentOutfitType(PlayerOutfitType value) { }
- Offset 0x21B14C4 / RVA 0x21B54C4: public float get_CalculatedAlpha() { }
- Offset 0x21B14CC / RVA 0x21B54CC: public float get_FlashlightAngle() { }
- Offset 0x21B14D4 / RVA 0x21B54D4: public void set_FlashlightAngle(float value) { }
- Offset 0x21B14EC / RVA 0x21B54EC: public bool get_PhantomFadeActive() { }
- Offset 0x21B07F8 / RVA 0x21B47F8: public NetworkedPlayerInfo get_Data() { }
- Offset 0x21B1500 / RVA 0x21B5500: public void SetKillTimer(float time) { }
- Offset 0x21B167C / RVA 0x21B567C: public bool get_Visible() { }
- Offset 0x21B16B0 / RVA 0x21B56B0: public void set_Visible(bool value) { }
- Offset 0x21B1A28 / RVA 0x21B5A28: public PlayerBodyTypes get_BodyType() { }
- Offset 0x21B1A8C / RVA 0x21B5A8C: private void Awake() { }
- Offset 0x21B1CB0 / RVA 0x21B5CB0: private void OnEnable() { }
- Offset 0x21B1FFC / RVA 0x21B5FFC: private void OnDisable() { }
- Offset 0x21B2110 / RVA 0x21B6110: private IEnumerator AssertWithTimeout(Func<bool> assertion, Action onTimeout, float timeoutInSeconds) { }
- Offset 0x21B21D0 / RVA 0x21B61D0: private IEnumerator Start() { }
- Offset 0x21B2264 / RVA 0x21B6264: private IEnumerator ClientInitialize() { }
- Offset 0x21B22F8 / RVA 0x21B62F8: public override void OnDestroy() { }
- Offset 0x21B238C / RVA 0x21B638C: private void FixedUpdate() { }
- Offset 0x21B37B8 / RVA 0x21B77B8: public void AnimateCustom(AnimationClip anim) { }
- Offset 0x21B3808 / RVA 0x21B7808: public void OnGameStart() { }
- Offset 0x21B3924 / RVA 0x21B7924: public void OnGameEnd() { }
- Offset 0x21B3944 / RVA 0x21B7944: public void UseClosest() { }
- Offset 0x21B3A3C / RVA 0x21B7A3C: public void RegisterVisibilityItem(IPlayerVisibleItem obj) { }
- Offset 0x21B3CB0 / RVA 0x21B7CB0: public void UnregisterVisibilityItem(IPlayerVisibleItem obj) { }
- Offset 0x21B3D08 / RVA 0x21B7D08: public void TryPet() { }
- Offset 0x21B3FE0 / RVA 0x21B7FE0: public void ReportClosest() { }
- Offset 0x21B4214 / RVA 0x21B8214: public void PlayStepSound() { }
- Offset 0x21B4568 / RVA 0x21B8568: private void SetScanner(bool on, byte cnt) { }
- Offset 0x21B36C0 / RVA 0x21B76C0: public Vector2 GetTruePosition() { }
- Offset 0x21B45F0 / RVA 0x21B85F0: public void SetTasks(List<NetworkedPlayerInfo.TaskInfo> tasks) { }
- Offset 0x21B4610 / RVA 0x21B8610: private IEnumerator CoSetTasks(List<NetworkedPlayerInfo.TaskInfo> tasks) { }
- Offset 0x21B46C0 / RVA 0x21B86C0: public PlayerTask AddSystemTask(SystemTypes system) { }
- Offset 0x21B4840 / RVA 0x21B8840: public void RemoveTask(PlayerTask task) { }
- Offset 0x21B4958 / RVA 0x21B8958: public void ClearTasks() { }
- Offset 0x21B4A74 / RVA 0x21B8A74: public void Die(DeathReason reason, bool assignGhostRole) { }
- Offset 0x21B529C / RVA 0x21B929C: public void Revive() { }

## AmongUsClient
public class AmongUsClient : InnerNetClient // TypeDefIndex: 1169
### Fields
- 0x0: public static AmongUsClient Instance
- 0x120: public string OnlineScene
- 0x128: public string MainMenuScene
- 0x130: public GameData GameDataPrefab
- 0x138: public VoteBanSystem VoteBanPrefab
- 0x140: public PlayerControl PlayerPrefab
- 0x148: public List<AssetReference> ShipPrefabs
- 0x150: public int TutorialMapId
- 0x154: public float SpawnRadius
- 0x158: public DiscoveryState discoverState
- 0x160: public List<IDisconnectHandler> DisconnectHandlers
- 0x168: public List<IGameListHandler> GameListHandlers
- 0x170: public CrossplayPrivilegeErrorType CrossplayPrivilegeError
- 0x174: public AmongUsClient.MainMenuTarget MenuTarget
- 0x178: private readonly Logger logger
- 0x180: private AsyncOperationHandle<GameObject> ShipLoadingAsyncHandle
- 0x20: public void Awake() { }

	// RVA: 0x213D1E8 Offset: 0x21391E8 VA: 0x213D1E8
	public void StartGame() { }

	// RVA: 0x213D204 Offset: 0x2139204 VA: 0x213D204 Slot: 6
	public override void Update() { }

	// RVA: 0x213D20C Offset: 0x213920C VA: 0x213D20C
	public void ExitGame(DisconnectReasons reason) { }

	// RVA: 0x213D7C4 Offset: 0x21397C4 VA: 0x213D7C4 Slot: 20
	protected override void OnGetGameList(InnerNetClient.TotalGameData totalGames, HttpMatchmakerManager.FindGamesListFilteredResponse gamesResponse) { }

	// RVA: 0x213D9D4 Offset: 0x21399D4 VA: 0x213D9D4 Slot: 17
	protected override void OnReportedPlayer(ReportOutcome outcome, int clientId, string playerName, ReportReasons reason) { }

	// RVA: 0x213D9D8 Offset: 0x21399D8 VA: 0x213D9D8 Slot: 8
	protected override void OnGameCreated(string gameIdString) { }

	// RVA: 0x213D9DC Offset: 0x21399DC VA: 0x213D9DC Slot: 10
	protected override void OnWaitForHost(string gameIdString) { }

	// RVA: 0x213D564 Offset: 0x2139564 VA: 0x213D564
	protected void AbortLoadingAssets() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoStartGameHost>d__28))]
	// RVA: 0x213DAAC Offset: 0x2139AAC VA: 0x213DAAC
	protected IEnumerator CoStartGameHost() { }

	// RVA: 0x213DB40 Offset: 0x2139B40 VA: 0x213DB40
	public void KickNotJoinedPlayers() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoStartGameClient>d__30))]
	// RVA: 0x213DCF0 Offset: 0x2139CF0 VA: 0x213DCF0
	protected IEnumerator CoStartGameClient() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoWaitForDisconnect>d__31))]
	// RVA: 0x213DD84 Offset: 0x2139D84 VA: 0x213DD84
	private IEnumerator CoWaitForDisconnect() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoStartGame>d__32))]
	// RVA: 0x213DE18 Offset: 0x2139E18 VA: 0x213DE18 Slot: 11
	protected override IEnumerator CoStartGame() { }

	// RVA: 0x213DEAC Offset: 0x2139EAC VA: 0x213DEAC Slot: 13
	protected override void OnBecomeHost() { }

	// RVA: 0x213E0E4 Offset: 0x213A0E4 VA: 0x213E0E4 Slot: 12
	protected override void OnGameEnd(EndGameResult endGameResult) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoEndGame>d__35))]
	// RVA: 0x213EB08 Offset: 0x213AB08 VA: 0x213EB08
	public IEnumerator CoEndGame() { }

	// RVA: 0x213EB88 Offset: 0x213AB88 VA: 0x213EB88 Slot: 14
	protected override void OnPlayerJoined(ClientData data) { }

	// RVA: 0x213EC50 Offset: 0x213AC50 VA: 0x213EC50 Slot: 9
	protected override void OnGameJoined(string gameIdString) { }

	// RVA: 0x213EE68 Offset: 0x213AE68 VA: 0x213EE68 Slot: 16
	protected override void OnPlayerLeft(ClientData data, DisconnectReasons reason) { }

	// RVA: 0x213F668 Offset: 0x213B668 VA: 0x213F668 Slot: 19
	protected override void PreDisconnectInternal() { }

	// RVA: 0x213F67C Offset: 0x213B67C VA: 0x213F67C Slot: 18
	protected override void OnDisconnected() { }

	// RVA: 0x213D61C Offset: 0x213961C VA: 0x213D61C
	private void DestroyPlayerInfoObjects() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoOnPlayerChangedScene>d__42))]
	// RVA: 0x213F730 Offset: 0x213B730 VA: 0x213F730 Slot: 15
	protected override IEnumerator CoOnPlayerChangedScene(ClientData client, string currentScene) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CreatePlayer>d__43))]
	// RVA: 0x213E05C Offset: 0x213A05C VA: 0x213E05C
	private IEnumerator CreatePlayer(ClientData clientData) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoBroadcastManager>d__44))]
	// RVA: 0x213F81C Offset: 0x213B81C VA: 0x213F81C
	private IEnumerator CoBroadcastManager() { }

	// RVA: 0x213F8B0 Offset: 0x213B8B0 VA: 0x213F8B0 Slot: 7
	protected override void OnApplicationPause(bool pause) { }

	// RVA: 0x213F9C4 Offset: 0x213B9C4 VA: 0x213F9C4
	public void CheckOnlinePermissions(Action success, Action failure, Action loadingCallback, bool checkOnline = True) { }

	[RuntimeInitializeOnLoadMethod(3)]
	// RVA: 0x213FADC Offset: 0x213BADC VA: 0x213FADC
	private static void InitSceneChangeListener() { }

	// RVA: 0x213FBD0 Offset: 0x213BBD0 VA: 0x213FBD0
	private static void OnActiveSceneChange(Scene from, Scene to) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoCreateOnlineGame>d__49))]
	// RVA: 0x213FDA0 Offset: 0x213BDA0 VA: 0x213FDA0
	public IEnumerator CoCreateOnlineGame() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoJoinOnlinePublicGame>d__50))]
	// RVA: 0x213FE34 Offset: 0x213BE34 VA: 0x213FE34
	public IEnumerator CoJoinOnlinePublicGame(int gameId, string ipAddress, ushort port, AmongUsClient.MainMenuTarget targetMenu = 1) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoJoinOnlineGameFromListing>d__51))]
	// RVA: 0x213FF04 Offset: 0x213BF04 VA: 0x213FF04
	public IEnumerator CoJoinOnlineGameFromListing(GameListing game, string matchmakerToken) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoJoinOnlineGameFromCode>d__52))]
	// RVA: 0x213FFD4 Offset: 0x213BFD4 VA: 0x213FFD4
	public IEnumerator CoJoinOnlineGameFromCode(int gameId, bool fromEnterCode = False) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoFindGameInfoFromCode>d__53))]
	// RVA: 0x2140084 Offset: 0x213C084 VA: 0x2140084
	public IEnumerator CoFindGameInfoFromCode(int gameId, Action<HttpMatchmakerManager.FindGameByCodeResponse, string> callback) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoFindGameInfoFromCodeAndJoin>d__54))]
	// RVA: 0x214013C Offset: 0x213C13C VA: 0x214013C
	public IEnumerator CoFindGameInfoFromCodeAndJoin(int gameId) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoFindGame>d__55))]
	// RVA: 0x21401E0 Offset: 0x213C1E0 VA: 0x21401E0
	public IEnumerator CoFindGame() { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoConnectToGameServer>d__56))]
	// RVA: 0x2140274 Offset: 0x213C274 VA: 0x2140274
	private IEnumerator CoConnectToGameServer(MatchMakerModes mode, string ipAddress, ushort port, string matchmakerToken) { }

	[IteratorStateMachine(typeof(AmongUsClient.<CoJoinOnlineGameDirect>d__57))]
	// RVA: 0x2140350 Offset: 0x213C350 VA: 0x2140350
	private IEnumerator CoJoinOnlineGameDirect(int gameId, string ipAddress, ushort port, string matchmakerToken) { }

	// RVA: 0x214042C Offset: 0x213C42C VA: 0x214042C
	public void .ctor() { }
}

// Namespace: 
public class AprilFoolsModeToggleButton : MonoBehaviour // TypeDefIndex: 1170
{
	// Fields
	public ButtonRolloverHandler onButton
- 0x28: public ButtonRolloverHandler offButton
- 0x30: public SpriteRenderer onButtonSprite
- 0x38: public SpriteRenderer offButtonSprite
- 0x40: public TextMeshPro onText
- 0x48: public TextMeshPro offText
- 0x30: public void ToggleAprilFoolsMode(bool modeOn) { }

	// RVA: 0x2145BF4 Offset: 0x2141BF4 VA: 0x2145BF4
	private void Update() { }

	// RVA: 0x2145D98 Offset: 0x2141D98 VA: 0x2145D98
	public void .ctor() { }
}

// Namespace: 
public class ChatLanguageButton : PoolableBehavior // TypeDefIndex: 1171
{
	// Fields
	public TextMeshPro Text
- 0x38: public PassiveButton Button
- 0x40: private SpriteRenderer Background
- 0x48: private SpriteRenderer Check
- 0x10: public void SetSelected(bool selected) { }

	// RVA: 0x2145F08 Offset: 0x2141F08 VA: 0x2145F08 Slot: 4
	public override void Reset() { }

	// RVA: 0x2145F10 Offset: 0x2141F10 VA: 0x2145F10
	public void .ctor() { }
}

// Namespace: 
[CompilerGenerated]
private sealed class ChatLanguageMenu.<>c__DisplayClass6_0 // TypeDefIndex: 1172
{
	// Fields
	public uint lang
- 0x18: public ChatLanguageMenu <>4__this
- 0x20: public void .ctor() { }

	// RVA: 0x21467D0 Offset: 0x21427D0 VA: 0x21467D0
	internal void <OnEnable>b__0() { }
}

// Namespace: 
public class ChatLanguageMenu : MonoBehaviour // TypeDefIndex: 1173
{
	// Fields
	public CreateOptionsPicker Parent
- 0x28: public ObjectPoolBehavior ButtonPool
- 0x30: public UiElement BackButton
- 0x38: private UiElement defaultButtonSelected
- 0x40: private List<UiElement> controllerSelectable
### Methods (first 40)
- Offset 0x2139068 / RVA 0x213D068: public void Awake() { }
- Offset 0x21391E8 / RVA 0x213D1E8: public void StartGame() { }
- Offset 0x2139204 / RVA 0x213D204: public override void Update() { }
- Offset 0x213920C / RVA 0x213D20C: public void ExitGame(DisconnectReasons reason) { }
- Offset 0x21397C4 / RVA 0x213D7C4: protected override void OnGetGameList(InnerNetClient.TotalGameData totalGames, HttpMatchmakerManager.FindGamesListFilteredResponse gamesResponse) { }
- Offset 0x21399D4 / RVA 0x213D9D4: protected override void OnReportedPlayer(ReportOutcome outcome, int clientId, string playerName, ReportReasons reason) { }
- Offset 0x21399D8 / RVA 0x213D9D8: protected override void OnGameCreated(string gameIdString) { }
- Offset 0x21399DC / RVA 0x213D9DC: protected override void OnWaitForHost(string gameIdString) { }
- Offset 0x2139564 / RVA 0x213D564: protected void AbortLoadingAssets() { }
- Offset 0x2139AAC / RVA 0x213DAAC: protected IEnumerator CoStartGameHost() { }
- Offset 0x2139B40 / RVA 0x213DB40: public void KickNotJoinedPlayers() { }
- Offset 0x2139CF0 / RVA 0x213DCF0: protected IEnumerator CoStartGameClient() { }
- Offset 0x2139D84 / RVA 0x213DD84: private IEnumerator CoWaitForDisconnect() { }
- Offset 0x2139E18 / RVA 0x213DE18: protected override IEnumerator CoStartGame() { }
- Offset 0x2139EAC / RVA 0x213DEAC: protected override void OnBecomeHost() { }
- Offset 0x213A0E4 / RVA 0x213E0E4: protected override void OnGameEnd(EndGameResult endGameResult) { }
- Offset 0x213AB08 / RVA 0x213EB08: public IEnumerator CoEndGame() { }
- Offset 0x213AB88 / RVA 0x213EB88: protected override void OnPlayerJoined(ClientData data) { }
- Offset 0x213AC50 / RVA 0x213EC50: protected override void OnGameJoined(string gameIdString) { }
- Offset 0x213AE68 / RVA 0x213EE68: protected override void OnPlayerLeft(ClientData data, DisconnectReasons reason) { }
- Offset 0x213B668 / RVA 0x213F668: protected override void PreDisconnectInternal() { }
- Offset 0x213B67C / RVA 0x213F67C: protected override void OnDisconnected() { }
- Offset 0x213961C / RVA 0x213D61C: private void DestroyPlayerInfoObjects() { }
- Offset 0x213B730 / RVA 0x213F730: protected override IEnumerator CoOnPlayerChangedScene(ClientData client, string currentScene) { }
- Offset 0x213A05C / RVA 0x213E05C: private IEnumerator CreatePlayer(ClientData clientData) { }
- Offset 0x213B81C / RVA 0x213F81C: private IEnumerator CoBroadcastManager() { }
- Offset 0x213B8B0 / RVA 0x213F8B0: protected override void OnApplicationPause(bool pause) { }
- Offset 0x213B9C4 / RVA 0x213F9C4: public void CheckOnlinePermissions(Action success, Action failure, Action loadingCallback, bool checkOnline = True) { }
- Offset 0x213BADC / RVA 0x213FADC: private static void InitSceneChangeListener() { }
- Offset 0x213BBD0 / RVA 0x213FBD0: private static void OnActiveSceneChange(Scene from, Scene to) { }
- Offset 0x213BDA0 / RVA 0x213FDA0: public IEnumerator CoCreateOnlineGame() { }
- Offset 0x213BE34 / RVA 0x213FE34: public IEnumerator CoJoinOnlinePublicGame(int gameId, string ipAddress, ushort port, AmongUsClient.MainMenuTarget targetMenu = 1) { }
- Offset 0x213BF04 / RVA 0x213FF04: public IEnumerator CoJoinOnlineGameFromListing(GameListing game, string matchmakerToken) { }
- Offset 0x213BFD4 / RVA 0x213FFD4: public IEnumerator CoJoinOnlineGameFromCode(int gameId, bool fromEnterCode = False) { }
- Offset 0x213C084 / RVA 0x2140084: public IEnumerator CoFindGameInfoFromCode(int gameId, Action<HttpMatchmakerManager.FindGameByCodeResponse, string> callback) { }
- Offset 0x213C13C / RVA 0x214013C: public IEnumerator CoFindGameInfoFromCodeAndJoin(int gameId) { }
- Offset 0x213C1E0 / RVA 0x21401E0: public IEnumerator CoFindGame() { }
- Offset 0x213C274 / RVA 0x2140274: private IEnumerator CoConnectToGameServer(MatchMakerModes mode, string ipAddress, ushort port, string matchmakerToken) { }
- Offset 0x213C350 / RVA 0x2140350: private IEnumerator CoJoinOnlineGameDirect(int gameId, string ipAddress, ushort port, string matchmakerToken) { }
- Offset 0x213C42C / RVA 0x214042C: public void .ctor() { }

## GameData
public class GameData : MonoBehaviour, IDisconnectHandler // TypeDefIndex: 695
### Fields
- 0x0: public static GameData Instance
- 0x8: public static float TimeGameStarted
- 0xC: public static float TimeLastMeetingStarted
- 0x10: public static int MeetingCount
- 0x14: public static int RoundsPlayedInSession
- 0x18: public static DeathReason LastDeathReason
- 0x20: public List<NetworkedPlayerInfo> AllPlayers
- 0x28: private List<NetworkedPlayerInfo> PlayerQueue
- 0x30: public int TotalTasks
- 0x34: public int CompletedTasks
- 0x38: public RoleBehaviour DefaultRole
- 0x40: public NetworkedPlayerInfo PlayerInfoPrefab
- 0x10: public int get_PlayerCount() { }

	// RVA: 0x24141A8 Offset: 0x24101A8 VA: 0x24141A8 Slot: 4
	public bool get_IsPersistent() { }

	// RVA: 0x24141B0 Offset: 0x24101B0 VA: 0x24141B0
	public static void DestroyInstance() { }

	// RVA: 0x2414278 Offset: 0x2410278 VA: 0x2414278
	public static void OnMeetingStart() { }

	// RVA: 0x24142C8 Offset: 0x24102C8 VA: 0x24142C8
	public static void OnGameEnd() { }

	// RVA: 0x2414340 Offset: 0x2410340 VA: 0x2414340
	public static void OnDisconnected() { }

	// RVA: 0x2414388 Offset: 0x2410388 VA: 0x2414388
	public void Awake() { }

	// RVA: 0x2414588 Offset: 0x2410588 VA: 0x2414588
	public NetworkedPlayerInfo GetHost() { }

	// RVA: 0x241463C Offset: 0x241063C VA: 0x241463C
	public bool HasPlayer(ClientData client) { }

	// RVA: 0x2414784 Offset: 0x2410784 VA: 0x2414784
	public sbyte GetPlayerIdFromClient(ClientData client) { }

	// RVA: 0x241481C Offset: 0x241081C VA: 0x241481C
	public sbyte GetAvailableId() { }

	// RVA: 0x24146B8 Offset: 0x24106B8 VA: 0x24146B8
	public NetworkedPlayerInfo GetPlayerByClient(ClientData client) { }

	// RVA: 0x2414974 Offset: 0x2410974 VA: 0x2414974
	public NetworkedPlayerInfo GetPlayerById(byte id) { }

	// RVA: 0x2414A44 Offset: 0x2410A44 VA: 0x2414A44
	public NetworkedPlayerInfo AddDummy(PlayerControl pc) { }

	// RVA: 0x2414C1C Offset: 0x2410C1C VA: 0x2414C1C
	public NetworkedPlayerInfo AddPlayer(PlayerControl pc, ClientData client) { }

	// RVA: 0x2414E84 Offset: 0x2410E84 VA: 0x2414E84
	public bool IsProcessingInfo(NetworkedPlayerInfo info) { }

	// RVA: 0x2414EDC Offset: 0x2410EDC VA: 0x2414EDC
	public void DirtyAllData() { }

	// RVA: 0x2414D20 Offset: 0x2410D20 VA: 0x2414D20
	public void AddPlayerInfo(NetworkedPlayerInfo info) { }

	// RVA: 0x2415060 Offset: 0x2411060 VA: 0x2415060
	public bool RemovePlayer(byte playerId) { }

	// RVA: 0x2415180 Offset: 0x2411180 VA: 0x2415180
	public void RecomputeTaskCounts() { }

	// RVA: 0x24153D0 Offset: 0x24113D0 VA: 0x24153D0
	public void TutOnlyRemoveTask(byte playerId, uint taskId) { }

	// RVA: 0x2415520 Offset: 0x2411520 VA: 0x2415520
	public uint TutOnlyAddTask(byte playerId) { }

	// RVA: 0x2415734 Offset: 0x2411734 VA: 0x2415734
	public void CompleteTask(PlayerControl pc, uint taskId) { }

	// RVA: 0x2415848 Offset: 0x2411848 VA: 0x2415848
	public void RemoveDisconnectedPlayers() { }

	// RVA: 0x24159C8 Offset: 0x24119C8 VA: 0x24159C8 Slot: 5
	public void HandleDisconnect(PlayerControl player, DisconnectReasons reason) { }

	// RVA: 0x2415BE4 Offset: 0x2411BE4 VA: 0x2415BE4
	private void ShowNotification(string playerName, DisconnectReasons reason) { }

	// RVA: 0x2415FE0 Offset: 0x2411FE0 VA: 0x2415FE0 Slot: 6
	public void HandleDisconnect() { }

	// RVA: 0x2416134 Offset: 0x2412134 VA: 0x2416134
	public void .ctor() { }
}

// Namespace: 
public class NetworkedPlayerInfo.TaskInfo // TypeDefIndex: 696
{
	// Fields
	public uint Id
- 0x14: public byte TypeId
- 0x15: public bool Complete
- 0x10: public int ColorId
- 0x18: public string HatId
- 0x20: public string PetId
- 0x28: public string SkinId
- 0x30: public string VisorId
- 0x38: public string NamePlateId
- 0x40: public string PlayerName
- 0x48: public byte HatSequenceId
- 0x49: public byte PetSequenceId
- 0x4A: public byte SkinSequenceId
- 0x4B: public byte VisorSequenceId
- 0x4C: public byte NamePlateSequenceId
- 0x0: public bool get_IsIncomplete() { }

	// RVA: 0x24181F4 Offset: 0x24141F4 VA: 0x24181F4 Slot: 3
	public override string ToString() { }

	// RVA: 0x2416E68 Offset: 0x2412E68 VA: 0x2416E68
	public void Serialize(MessageWriter writer) { }

	// RVA: 0x2417468 Offset: 0x2413468 VA: 0x2417468
	public void Deserialize(MessageReader reader) { }

	// RVA: 0x2417394 Offset: 0x2413394 VA: 0x2417394
	public void .ctor() { }
}

// Namespace: 
[CompilerGenerated]
[Serializable]
private sealed class NetworkedPlayerInfo.<>c // TypeDefIndex: 698
{
	// Fields
	public static readonly NetworkedPlayerInfo.<>c <>9
- 0x8: public static Func<NetworkedPlayerInfo.PlayerOutfit, bool> <>9__14_0
- 0x10: private static void .cctor() { }

	// RVA: 0x241849C Offset: 0x241449C VA: 0x241849C
	public void .ctor() { }

	// RVA: 0x24184A4 Offset: 0x24144A4 VA: 0x24184A4
	internal bool <get_IsIncomplete>b__14_0(NetworkedPlayerInfo.PlayerOutfit outfit) { }
}

// Namespace: 
[CompilerGenerated]
private sealed class NetworkedPlayerInfo.<CoCensorNameAsync>d__38 : IEnumerator<object>, IEnumerator, IDisposable // TypeDefIndex: 699
{
	// Fields
	private int <>1__state
- 0x18: private object <>2__current
- 0x20: public NetworkedPlayerInfo <>4__this
- 0x28: public Action callback
- 0x30: private int <attempts>5__2
- 0x10: public void .ctor(int <>1__state) { }

	[DebuggerHidden]
	// RVA: 0x24184B8 Offset: 0x24144B8 VA: 0x24184B8 Slot: 5
	private void System.IDisposable.Dispose() { }

	// RVA: 0x24184BC Offset: 0x24144BC VA: 0x24184BC Slot: 6
	private bool MoveNext() { }

	[DebuggerHidden]
	// RVA: 0x2418744 Offset: 0x2414744 VA: 0x2418744 Slot: 4
	private object System.Collections.Generic.IEnumerator<System.Object>.get_Current() { }

	[DebuggerHidden]
	// RVA: 0x241874C Offset: 0x241474C VA: 0x241874C Slot: 8
	private void System.Collections.IEnumerator.Reset() { }

	[DebuggerHidden]
	// RVA: 0x2418784 Offset: 0x2414784 VA: 0x2418784 Slot: 7
	private object System.Collections.IEnumerator.get_Current() { }
}

// Namespace: 
[CompilerGenerated]
private sealed class NetworkedPlayerInfo.<CoUpdateColor>d__48 : IEnumerator<object>, IEnumerator, IDisposable // TypeDefIndex: 700
{
	// Fields
	private int <>1__state
- 0x18: private object <>2__current
- 0x20: public NetworkedPlayerInfo <>4__this
- 0x28: public int colorId
- 0x2C: private int <attempts>5__2
### Methods (first 40)
- Offset 0x2410160 / RVA 0x2414160: public int get_PlayerCount() { }
- Offset 0x24101A8 / RVA 0x24141A8: public bool get_IsPersistent() { }
- Offset 0x24101B0 / RVA 0x24141B0: public static void DestroyInstance() { }
- Offset 0x2410278 / RVA 0x2414278: public static void OnMeetingStart() { }
- Offset 0x24102C8 / RVA 0x24142C8: public static void OnGameEnd() { }
- Offset 0x2410340 / RVA 0x2414340: public static void OnDisconnected() { }
- Offset 0x2410388 / RVA 0x2414388: public void Awake() { }
- Offset 0x2410588 / RVA 0x2414588: public NetworkedPlayerInfo GetHost() { }
- Offset 0x241063C / RVA 0x241463C: public bool HasPlayer(ClientData client) { }
- Offset 0x2410784 / RVA 0x2414784: public sbyte GetPlayerIdFromClient(ClientData client) { }
- Offset 0x241081C / RVA 0x241481C: public sbyte GetAvailableId() { }
- Offset 0x24106B8 / RVA 0x24146B8: public NetworkedPlayerInfo GetPlayerByClient(ClientData client) { }
- Offset 0x2410974 / RVA 0x2414974: public NetworkedPlayerInfo GetPlayerById(byte id) { }
- Offset 0x2410A44 / RVA 0x2414A44: public NetworkedPlayerInfo AddDummy(PlayerControl pc) { }
- Offset 0x2410C1C / RVA 0x2414C1C: public NetworkedPlayerInfo AddPlayer(PlayerControl pc, ClientData client) { }
- Offset 0x2410E84 / RVA 0x2414E84: public bool IsProcessingInfo(NetworkedPlayerInfo info) { }
- Offset 0x2410EDC / RVA 0x2414EDC: public void DirtyAllData() { }
- Offset 0x2410D20 / RVA 0x2414D20: public void AddPlayerInfo(NetworkedPlayerInfo info) { }
- Offset 0x2411060 / RVA 0x2415060: public bool RemovePlayer(byte playerId) { }
- Offset 0x2411180 / RVA 0x2415180: public void RecomputeTaskCounts() { }
- Offset 0x24113D0 / RVA 0x24153D0: public void TutOnlyRemoveTask(byte playerId, uint taskId) { }
- Offset 0x2411520 / RVA 0x2415520: public uint TutOnlyAddTask(byte playerId) { }
- Offset 0x2411734 / RVA 0x2415734: public void CompleteTask(PlayerControl pc, uint taskId) { }
- Offset 0x2411848 / RVA 0x2415848: public void RemoveDisconnectedPlayers() { }
- Offset 0x24119C8 / RVA 0x24159C8: public void HandleDisconnect(PlayerControl player, DisconnectReasons reason) { }
- Offset 0x2411BE4 / RVA 0x2415BE4: private void ShowNotification(string playerName, DisconnectReasons reason) { }
- Offset 0x2411FE0 / RVA 0x2415FE0: public void HandleDisconnect() { }
- Offset 0x2412134 / RVA 0x2416134: public void .ctor() { }
- Offset 0x241172C / RVA 0x241572C: public void .ctor() { }
- Offset 0x2413EA4 / RVA 0x2417EA4: public void .ctor(byte typeId, uint id) { }
- Offset 0x2412F48 / RVA 0x2416F48: public void Serialize(MessageWriter writer) { }
- Offset 0x2413594 / RVA 0x2417594: public void Deserialize(MessageReader reader) { }
- Offset 0x241412C / RVA 0x241812C: public bool get_IsIncomplete() { }
- Offset 0x24141F4 / RVA 0x24181F4: public override string ToString() { }
- Offset 0x2412E68 / RVA 0x2416E68: public void Serialize(MessageWriter writer) { }
- Offset 0x2413468 / RVA 0x2417468: public void Deserialize(MessageReader reader) { }
- Offset 0x2413394 / RVA 0x2417394: public void .ctor() { }
- Offset 0x2414434 / RVA 0x2418434: private static void .cctor() { }
- Offset 0x241449C / RVA 0x241849C: public void .ctor() { }
- Offset 0x24144A4 / RVA 0x24184A4: internal bool <get_IsIncomplete>b__14_0(NetworkedPlayerInfo.PlayerOutfit outfit) { }

## ShipStatus
public class ShipStatus : InnerNetObject // TypeDefIndex: 1808
### Fields
- 0x0: public static ShipStatus Instance
- 0x38: public Color CameraColor
- 0x48: public float MaxLightRadius
- 0x4C: public float MinLightRadius
- 0x50: public float MapScale
- 0x58: public MapBehaviour MapPrefab
- 0x60: public ExileController ExileCutscenePrefab
- 0x68: public MeetingCalledAnimation EmergencyOverlay
- 0x70: public MeetingCalledAnimation ReportOverlay
- 0x78: public Sprite MeetingBackground
- 0x80: public Sprite BrokenEmergencyButton
- 0x88: public SystemConsole EmergencyButton
- 0x90: public Vector2 InitialSpawnCenter
- 0x98: public Vector2 MeetingSpawnCenter
- 0xA0: public Vector2 MeetingSpawnCenter2
- 0xA8: public float SpawnRadius
- 0xB0: public NormalPlayerTask[] CommonTasks
- 0xB8: public NormalPlayerTask[] LongTasks
- 0xC0: public NormalPlayerTask[] ShortTasks
- 0xC8: public PlayerTask[] SpecialTasks
- 0xD0: public Transform[] DummyLocations
- 0xD8: public SurvCamera[] AllCameras
- 0xE0: public OpenableDoor[] AllDoors
- 0xE8: public Console[] AllConsoles
- 0xF0: public Ladder[] Ladders
- 0xF8: public Dictionary<SystemTypes, ISystemType> Systems
- 0x100: public StringNames[] SystemNames
- 0x108: public StringNames[] ExtraTaskNames
- 0x110: private IStepWatcher[] <AllStepWatchers>k__BackingField
- 0x118: private PlainShipRoom[] <AllRooms>k__BackingField
- 0x120: private Dictionary<SystemTypes, PlainShipRoom> <FastRooms>k__BackingField
- 0x128: private Vent[] <AllVents>k__BackingField
- 0x130: public AudioClip SabotageSound
- 0x138: public AnimationClip[] WeaponFires
- 0x140: public SpriteAnim WeaponsImage
- 0x148: public AudioClip[] VentMoveSounds
- 0x150: public AudioClip VentEnterSound
- 0x158: public AudioClip VentExitSound
- 0x160: public AnimationClip HatchActive
- 0x168: public SpriteAnim Hatch
- 0x170: public ParticleSystem HatchParticles
- 0x178: public AnimationClip ShieldsActive
- 0x180: public SpriteAnim[] ShieldsImages
- 0x188: public SpriteRenderer ShieldBorder
- 0x190: public Sprite ShieldBorderOn
- 0x198: public MedScannerBehaviour MedScanner
- 0x1A0: private int WeaponFireIdx
- 0x1A4: public float Timer
- 0x1A8: public float EmergencyCooldown
- 0x1AC: public ShipStatus.MapType Type
- 0x1B0: private float <HideCountdown>k__BackingField
- 0x1B8: private CosmeticsCache <CosmeticsCache>k__BackingField
- 0x1C0: protected readonly Logger logger
- 0x1C8: private int numScans
### Methods (first 40)
- Offset 0x22263FC / RVA 0x222A3FC: public IStepWatcher[] get_AllStepWatchers() { }
- Offset 0x2226404 / RVA 0x222A404: private void set_AllStepWatchers(IStepWatcher[] value) { }
- Offset 0x2226414 / RVA 0x222A414: public PlainShipRoom[] get_AllRooms() { }
- Offset 0x222641C / RVA 0x222A41C: private void set_AllRooms(PlainShipRoom[] value) { }
- Offset 0x222642C / RVA 0x222A42C: public Dictionary<SystemTypes, PlainShipRoom> get_FastRooms() { }
- Offset 0x2226434 / RVA 0x222A434: private void set_FastRooms(Dictionary<SystemTypes, PlainShipRoom> value) { }
- Offset 0x2226444 / RVA 0x222A444: public Vent[] get_AllVents() { }
- Offset 0x222644C / RVA 0x222A44C: private void set_AllVents(Vent[] value) { }
- Offset 0x222645C / RVA 0x222A45C: public float get_HideCountdown() { }
- Offset 0x2226464 / RVA 0x222A464: public void set_HideCountdown(float value) { }
- Offset 0x222646C / RVA 0x222A46C: public CosmeticsCache get_CosmeticsCache() { }
- Offset 0x2226474 / RVA 0x222A474: public void set_CosmeticsCache(CosmeticsCache value) { }
- Offset 0x2226484 / RVA 0x222A484: public override bool get_IsDirty() { }
- Offset 0x2226660 / RVA 0x222A660: protected virtual void OnEnable() { }
- Offset 0x2226E00 / RVA 0x222AE00: public virtual void RepairCriticalSabotages() { }
- Offset 0x2226ECC / RVA 0x222AECC: private void Awake() { }
- Offset 0x22276E8 / RVA 0x222B6E8: protected virtual void Start() { }
- Offset 0x2227818 / RVA 0x222B818: public override void OnDestroy() { }
- Offset 0x2227BD4 / RVA 0x222BBD4: public virtual void SpawnPlayer(PlayerControl player, int numPlayers, bool initialSpawn) { }
- Offset 0x2227D54 / RVA 0x222BD54: public void StartShields() { }
- Offset 0x2227DCC / RVA 0x222BDCC: public void FireWeapon() { }
- Offset 0x2227EA4 / RVA 0x222BEA4: public NormalPlayerTask GetTaskById(byte idx) { }
- Offset 0x222802C / RVA 0x222C02C: public PlayerTask[] GetAllTasks() { }
- Offset 0x22280A4 / RVA 0x222C0A4: public bool HasTaskTypes(Type[] types) { }
- Offset 0x2227424 / RVA 0x222B424: private void InitializeExtraTaskNames() { }
- Offset 0x22281EC / RVA 0x222C1EC: public void OpenHatch() { }
- Offset 0x222829C / RVA 0x222C29C: public void CloseDoorsOfType(SystemTypes room) { }
- Offset 0x2225DE0 / RVA 0x2229DE0: public void UpdateSystem(SystemTypes systemType, PlayerControl player, byte amount) { }
- Offset 0x2228428 / RVA 0x222C428: public void UpdateSystem(SystemTypes systemType, PlayerControl player, MessageReader msgReader) { }
- Offset 0x2227344 / RVA 0x222B344: private void AssignTaskIndexes() { }
- Offset 0x22285C4 / RVA 0x222C5C4: public virtual void OnMeetingCalled() { }
- Offset 0x22285C8 / RVA 0x222C5C8: public virtual void StartSFX() { }
- Offset 0x22285CC / RVA 0x222C5CC: public virtual IEnumerator PrespawnStep() { }
- Offset 0x222864C / RVA 0x222C64C: public void Begin() { }
- Offset 0x2228FD8 / RVA 0x222CFD8: private void AddTasksFromList(ref int start, int count, List<byte> tasks, HashSet<TaskTypes> usedTaskTypes, List<NormalPlayerTask> unusedTasks) { }
- Offset 0x2229368 / RVA 0x222D368: public void FixedUpdate() { }
- Offset 0x222962C / RVA 0x222D62C: public virtual float CalculateLightRadius(NetworkedPlayerInfo player) { }
- Offset 0x22298F4 / RVA 0x222D8F4: public void StartMeeting(PlayerControl reporter, NetworkedPlayerInfo target) { }
- Offset 0x222999C / RVA 0x222D99C: public PlayerTask GetSabotageTask(SystemTypes system) { }
- Offset 0x2229914 / RVA 0x222D914: public IEnumerator CoStartMeeting(PlayerControl reporter, NetworkedPlayerInfo target) { }

## HudManager
public class HudManager : DestroyableSingleton<HudManager> // TypeDefIndex: 928
### Fields
- 0x28: public FollowerCamera PlayerCam
- 0x30: public Camera UICamera
- 0x38: public MeetingHud MeetingPrefab
- 0x40: public KillButton KillButton
- 0x48: public AdminButton AdminButton
- 0x50: public SabotageButton SabotageButton
- 0x58: public VentButton ImpostorVentButton
- 0x60: public UseButton UseButton
- 0x68: public PetButton PetButton
- 0x70: public AbilityButton AbilityButton
- 0x78: public SecondaryAbilityButton SecondaryAbilityButton
- 0x80: public ReportButton ReportButton
- 0x88: public GameObject TaskStuff
- 0x90: public TaskPanelBehaviour TaskPanel
- 0x98: public CrewmatesKilledTracker CrewmatesKilled
- 0xA0: public ChatController Chat
- 0xA8: public DialogueBox Dialogue
- 0xB0: public Transform TaskCompleteOverlay
- 0xB8: private float taskDirtyTimer
- 0xC0: public MeshRenderer ShadowQuad
- 0xC8: public SpriteRenderer FullScreen
- 0xD0: private Coroutine <ReactorFlash>k__BackingField
- 0xD8: private Coroutine <OxyFlash>k__BackingField
- 0xE0: public PassiveButton MapButton
- 0xE8: public GameObject MapButtonGlyph
- 0xF0: public KillOverlay KillOverlay
- 0xF8: public IVirtualJoystick joystick
- 0x100: public VirtualJoystick joystickR
- 0x108: public MonoBehaviour[] Joysticks
- 0x110: public MonoBehaviour RightVJoystick
- 0x118: public Collider2D LeftStickDeadZone
- 0x120: public Collider2D RightStickDeadZone
- 0x128: public DiscussBehaviour discussEmblem
- 0x130: public ShhhBehaviour shhhEmblem
- 0x138: public IntroCutscene IntroPrefab
- 0x140: public OptionsMenuBehaviour GameMenu
- 0x148: public NotificationPopper Notifier
- 0x150: public RoomTracker roomTracker
- 0x158: public AudioClip TaskCompleteSound
- 0x160: public AudioClip TaskUpdateSound
- 0x168: public Transform consoleUIRoot
- 0x170: public GameObject[] consoleUIObjects
- 0x178: public GameObject menuNavigationPrompts
- 0x180: public GameObject GameLoadAnimation
- 0x188: public LobbyTimerExtensionUI LobbyTimerExtensionUI
- 0x190: public float consoleUIHorizontalShift
- 0x198: public GameObject playerListPrompt
- 0x1A0: public AlertFlash AlertFlash
- 0x1A8: public DangerMeter DangerMeter
- 0x1B0: public GameObject SettingsButton
- 0x1B8: private StringBuilder tasksString
- 0x1C0: private bool <IsIntroDisplayed>k__BackingField
- 0x0: public int value__
### Methods (first 40)
- Offset 0x20F837C / RVA 0x20FC37C: public Coroutine get_ReactorFlash() { }
- Offset 0x20F8384 / RVA 0x20FC384: public void set_ReactorFlash(Coroutine value) { }
- Offset 0x20F838C / RVA 0x20FC38C: public Coroutine get_OxyFlash() { }
- Offset 0x20F8394 / RVA 0x20FC394: public void set_OxyFlash(Coroutine value) { }
- Offset 0x20F839C / RVA 0x20FC39C: public bool get_IsIntroDisplayed() { }
- Offset 0x20F83A4 / RVA 0x20FC3A4: private void set_IsIntroDisplayed(bool value) { }
- Offset 0x20F83B0 / RVA 0x20FC3B0: public void Start() { }
- Offset 0x20F848C / RVA 0x20FC48C: public void OnGameStart() { }
- Offset 0x20F8538 / RVA 0x20FC538: public void ShowTaskComplete() { }
- Offset 0x20F8558 / RVA 0x20FC558: private IEnumerator CoTaskComplete() { }
- Offset 0x20F85CC / RVA 0x20FC5CC: public void SetJoystickSize(float size) { }
- Offset 0x20F8778 / RVA 0x20FC778: private void SetVirtualJoystickSize(VirtualJoystick jStick, float size, Vector2 dist) { }
- Offset 0x20F8820 / RVA 0x20FC820: public void SetTouchType(ControlTypes type) { }
- Offset 0x20F8D54 / RVA 0x20FCD54: public void InitMap() { }
- Offset 0x20F8EA0 / RVA 0x20FCEA0: public void DeselectMapButton() { }
- Offset 0x20F8EC0 / RVA 0x20FCEC0: public void ToggleMapVisible(MapOptions options) { }
- Offset 0x20F9028 / RVA 0x20FD028: public void SetHudActive(PlayerControl localPlayer, RoleBehaviour role, bool isActive) { }
- Offset 0x20F9708 / RVA 0x20FD708: public void UpdateVoteTimerText(StringNames key, int value) { }
- Offset 0x20F9724 / RVA 0x20FD724: public void SetHudActive(bool isActive) { }
- Offset 0x20F9670 / RVA 0x20FD670: public void ToggleRightJoystick(bool isActive) { }
- Offset 0x20F97B4 / RVA 0x20FD7B4: public void ToggleMapButton(bool isActive) { }
- Offset 0x20F97E4 / RVA 0x20FD7E4: public void ToggleMapButtonGlyph(bool isActive) { }
- Offset 0x20F9804 / RVA 0x20FD804: public void SetMapButtonEnabled(bool enabled) { }
- Offset 0x20F989C / RVA 0x20FD89C: public void ToggleSettingsButton(bool isActive) { }
- Offset 0x20F98BC / RVA 0x20FD8BC: public void Update() { }
- Offset 0x20F9F10 / RVA 0x20FDF10: public IEnumerator ShowEmblem(bool shhh) { }
- Offset 0x20F9F98 / RVA 0x20FDF98: public void StartReactorFlash() { }
- Offset 0x20FA05C / RVA 0x20FE05C: public void StartOxyFlash() { }
- Offset 0x20FA0AC / RVA 0x20FE0AC: public void ShowPopUp(string text) { }
- Offset 0x20FA0C4 / RVA 0x20FE0C4: public void StopReactorFlash() { }
- Offset 0x20FA340 / RVA 0x20FE340: public void StopOxyFlash() { }
- Offset 0x20FA3A8 / RVA 0x20FE3A8: public IEnumerator CoFadeFullScreen(Color source, Color target, float duration = 0.2, bool showLoader = False) { }
- Offset 0x20F9FE8 / RVA 0x20FDFE8: private IEnumerator CoReactorFlash() { }
- Offset 0x20FA48C / RVA 0x20FE48C: public IEnumerator CoShowIntro() { }
- Offset 0x20FA500 / RVA 0x20FE500: public void HideGameLoader() { }
- Offset 0x20FA520 / RVA 0x20FE520: public void OpenMeetingRoom(PlayerControl reporter) { }
- Offset 0x20FA73C / RVA 0x20FE73C: public void SetAlertOverlay(bool enabled) { }
- Offset 0x20FA75C / RVA 0x20FE75C: public override void OnDestroy() { }
- Offset 0x20FA7A4 / RVA 0x20FE7A4: public void NotifyOfDeath() { }
- Offset 0x20FA7BC / RVA 0x20FE7BC: public void NotifyOfDisconnect(PlayerControl pc) { }

## CustomNetworkTransform
public class CustomNetworkTransform : InnerNetObject // TypeDefIndex: 1054
### Fields
- 0x38: private PlayerControl myPlayer
- 0x40: private Rigidbody2D body
- 0x48: private Queue<Vector2> sendQueue
- 0x50: private Queue<Vector2> incomingPosQueue
- 0x58: private float rubberbandModifier
- 0x5C: private float idealSpeed
- 0x60: private bool isPaused
- 0x62: private ushort lastSequenceId
- 0x64: private Vector2 lastPosition
- 0x6C: private Vector2 lastPosSent
- 0x74: private Nullable<Vector2> tempSnapPosition
- 0x80: private ITransformGhost debugPopPositions
- 0x88: private ITransformGhost debugTargetPositions
- 0x90: private ITransformGhost debugTruePositions
- 0x98: private INetTransformLogger debugNetLogger
- 0x0: private void Awake() { }

	// RVA: 0x21230E0 Offset: 0x211F0E0 VA: 0x21230E0
	public void OnDrawGizmos() { }

	// RVA: 0x2123348 Offset: 0x211F348 VA: 0x2123348
	public void OnEnable() { }

	// RVA: 0x212340C Offset: 0x211F40C VA: 0x212340C
	public void SetPaused(bool isPaused) { }

	// RVA: 0x2123418 Offset: 0x211F418 VA: 0x2123418
	public void Halt() { }

	// RVA: 0x212360C Offset: 0x211F60C VA: 0x212360C
	public void RpcSnapTo(Vector2 position) { }

	// RVA: 0x212380C Offset: 0x211F80C VA: 0x212380C
	public void SnapTo(Vector2 position) { }

	// RVA: 0x2123818 Offset: 0x211F818 VA: 0x2123818
	public void ClearPositionQueues() { }

	// RVA: 0x2123880 Offset: 0x211F880 VA: 0x2123880
	private bool IsInMiddleOfAnimationThatMakesPlayerInvisible() { }

	// RVA: 0x2123458 Offset: 0x211F458 VA: 0x2123458
	private void SnapTo(Vector2 position, ushort minSid) { }

	// RVA: 0x212391C Offset: 0x211F91C VA: 0x212391C
	private void FixedUpdate() { }

	// RVA: 0x2123A94 Offset: 0x211FA94 VA: 0x2123A94
	private bool HasMoved() { }

	// RVA: 0x2124354 Offset: 0x2120354 VA: 0x2124354 Slot: 7
	public override void HandleRpc(byte callId, MessageReader reader) { }

	// RVA: 0x2124500 Offset: 0x2120500 VA: 0x2124500 Slot: 8
	public override void ClearOrDecrementDirt() { }

	// RVA: 0x2124514 Offset: 0x2120514 VA: 0x2124514 Slot: 9
	public override bool Serialize(MessageWriter writer, bool initialState) { }

	// RVA: 0x2124808 Offset: 0x2120808 VA: 0x2124808 Slot: 10
	public override void Deserialize(MessageReader reader, bool initialState) { }

	// RVA: 0x2123D9C Offset: 0x211FD9C VA: 0x2123D9C
	private void MoveTowardNextPoint() { }

	// RVA: 0x2123BA4 Offset: 0x211FBA4 VA: 0x2123BA4
	private void SkipExcessiveFrames() { }

	// RVA: 0x2124C14 Offset: 0x2120C14 VA: 0x2124C14
	private bool ShouldExtendCurrentFrame(Vector2 nextPos, Vector2 currentPos) { }

	// RVA: 0x2124CA8 Offset: 0x2120CA8 VA: 0x2124CA8
	private bool DidPassPosition(Vector2 nextPos, Vector2 lastPos, Vector2 currentPos) { }

	// RVA: 0x2123D00 Offset: 0x211FD00 VA: 0x2123D00
	private void SetMovementSmoothingModifier() { }

	// RVA: 0x2124DE8 Offset: 0x2120DE8 VA: 0x2124DE8
	public void .ctor() { }
}

// Namespace: 
public enum DisconnectReasons // TypeDefIndex: 1055
{
	// Fields
	public int value__
- 0x10: private readonly string <Name>k__BackingField
- 0x18: private readonly StringNames <TranslateName>k__BackingField
- 0x20: private readonly string <TargetServer>k__BackingField
- 0x28: public readonly string Fqdn
- 0x30: public readonly string DefaultIp
- 0x38: public readonly ushort Port
- 0x3A: public readonly bool UseDtls
- 0x40: private ServerInfo[] cachedServers
- 0x0: public string get_Name() { }

	// RVA: 0x2124F64 Offset: 0x2120F64 VA: 0x2124F64 Slot: 5
	public string get_PingServer() { }

	// RVA: 0x2124FF4 Offset: 0x2120FF4 VA: 0x2124FF4 Slot: 6
	public ServerInfo[] get_Servers() { }

	[CompilerGenerated]
	// RVA: 0x2125370 Offset: 0x2121370 VA: 0x2125370 Slot: 7
	public StringNames get_TranslateName() { }

	[CompilerGenerated]
	// RVA: 0x2125378 Offset: 0x2121378 VA: 0x2125378 Slot: 8
	public string get_TargetServer() { }

	// RVA: 0x2125380 Offset: 0x2121380 VA: 0x2125380
	public void .ctor(string fqdn, string name, StringNames translateName, string defaultIp, ushort port, bool useDtls = True) { }

	// RVA: 0x2125018 Offset: 0x2121018 VA: 0x2125018
	private void PopulateServers() { }

	// RVA: 0x2125424 Offset: 0x2121424 VA: 0x2125424
	private void .ctor(string fqdn, string name, StringNames translateName, ServerInfo[] servers) { }

	// RVA: 0x2125494 Offset: 0x2121494 VA: 0x2125494 Slot: 10
	public bool Validate() { }

	// RVA: 0x21254B4 Offset: 0x21214B4 VA: 0x21254B4 Slot: 9
	public IRegionInfo Duplicate() { }

	// RVA: 0x2125548 Offset: 0x2121548 VA: 0x2125548 Slot: 2
	public override int GetHashCode() { }

	// RVA: 0x2125568 Offset: 0x2121568 VA: 0x2125568 Slot: 0
	public override bool Equals(object obj) { }
}

// Namespace: 
[CompilerGenerated]
[Serializable]
private sealed class HttpUtils.<>c // TypeDefIndex: 1057
{
	// Fields
	public static readonly HttpUtils.<>c <>9
- 0x8: public static Func<KeyValuePair<string, object>, string> <>9__1_0
### Methods (first 40)
- Offset 0x211EFF0 / RVA 0x2122FF0: private void Awake() { }
- Offset 0x211F0E0 / RVA 0x21230E0: public void OnDrawGizmos() { }
- Offset 0x211F348 / RVA 0x2123348: public void OnEnable() { }
- Offset 0x211F40C / RVA 0x212340C: public void SetPaused(bool isPaused) { }
- Offset 0x211F418 / RVA 0x2123418: public void Halt() { }
- Offset 0x211F60C / RVA 0x212360C: public void RpcSnapTo(Vector2 position) { }
- Offset 0x211F80C / RVA 0x212380C: public void SnapTo(Vector2 position) { }
- Offset 0x211F818 / RVA 0x2123818: public void ClearPositionQueues() { }
- Offset 0x211F880 / RVA 0x2123880: private bool IsInMiddleOfAnimationThatMakesPlayerInvisible() { }
- Offset 0x211F458 / RVA 0x2123458: private void SnapTo(Vector2 position, ushort minSid) { }
- Offset 0x211F91C / RVA 0x212391C: private void FixedUpdate() { }
- Offset 0x211FA94 / RVA 0x2123A94: private bool HasMoved() { }
- Offset 0x2120354 / RVA 0x2124354: public override void HandleRpc(byte callId, MessageReader reader) { }
- Offset 0x2120500 / RVA 0x2124500: public override void ClearOrDecrementDirt() { }
- Offset 0x2120514 / RVA 0x2124514: public override bool Serialize(MessageWriter writer, bool initialState) { }
- Offset 0x2120808 / RVA 0x2124808: public override void Deserialize(MessageReader reader, bool initialState) { }
- Offset 0x211FD9C / RVA 0x2123D9C: private void MoveTowardNextPoint() { }
- Offset 0x211FBA4 / RVA 0x2123BA4: private void SkipExcessiveFrames() { }
- Offset 0x2120C14 / RVA 0x2124C14: private bool ShouldExtendCurrentFrame(Vector2 nextPos, Vector2 currentPos) { }
- Offset 0x2120CA8 / RVA 0x2124CA8: private bool DidPassPosition(Vector2 nextPos, Vector2 lastPos, Vector2 currentPos) { }
- Offset 0x211FD00 / RVA 0x2123D00: private void SetMovementSmoothingModifier() { }
- Offset 0x2120DE8 / RVA 0x2124DE8: public void .ctor() { }
- Offset 0x2120F5C / RVA 0x2124F5C: public string get_Name() { }
- Offset 0x2120F64 / RVA 0x2124F64: public string get_PingServer() { }
- Offset 0x2120FF4 / RVA 0x2124FF4: public ServerInfo[] get_Servers() { }
- Offset 0x2121370 / RVA 0x2125370: public StringNames get_TranslateName() { }
- Offset 0x2121378 / RVA 0x2125378: public string get_TargetServer() { }
- Offset 0x2121380 / RVA 0x2125380: public void .ctor(string fqdn, string name, StringNames translateName, string defaultIp, ushort port, bool useDtls = True) { }
- Offset 0x2121018 / RVA 0x2125018: private void PopulateServers() { }
- Offset 0x2121424 / RVA 0x2125424: private void .ctor(string fqdn, string name, StringNames translateName, ServerInfo[] servers) { }
- Offset 0x2121494 / RVA 0x2125494: public bool Validate() { }
- Offset 0x21214B4 / RVA 0x21254B4: public IRegionInfo Duplicate() { }
- Offset 0x2121548 / RVA 0x2125548: public override int GetHashCode() { }
- Offset 0x2121568 / RVA 0x2125568: public override bool Equals(object obj) { }
- Offset 0x2121778 / RVA 0x2125778: private static void .cctor() { }
- Offset 0x21217E0 / RVA 0x21257E0: public void .ctor() { }
- Offset 0x21217E8 / RVA 0x21257E8: internal string <ToUriQuery>b__1_0(KeyValuePair<string, object> kvp) { }
- Offset 0x2121640 / RVA 0x2125640: public static bool IsSuccess(long httpStatusCode) { }
- Offset 0x2121650 / RVA 0x2125650: public static string ToUriQuery(Dictionary<string, object> uriParams) { }
40938:	public const RpcCalls MurderPlayer = 12;
55904:	public void MurderPlayer(PlayerControl target, MurderResultFlags resultFlags) { }
56105:	public void CmdCheckMurder(PlayerControl target) { }
56111:	public void RpcMurderPlayer(PlayerControl target, bool didSucceed) { }

## Key PlayerControl method offsets

- // RVA: 0x21BD300 Offset: 0x21B9300 VA: 0x21BD300
  public void MurderPlayer(PlayerControl target, MurderResultFlags resultFlags) { }
- // RVA: 0x21BCAB8 Offset: 0x21B8AB8 VA: 0x21BCAB8
  public void RpcMurderPlayer(PlayerControl target, bool didSucceed) { }
- // RVA: 0x21C2CE8 Offset: 0x21BECE8 VA: 0x21C2CE8
  public void CmdCheckMurder(PlayerControl target) { }
- // RVA: 0x21C1CEC Offset: 0x21BDCEC VA: 0x21C1CEC
  public void RpcSetRole(RoleTypes roleType, bool canOverrideRole = False) { }
- // RVA: 0x21B47F8 Offset: 0x21B07F8 VA: 0x21B47F8
  public NetworkedPlayerInfo get_Data() { }
- // RVA: 0x21B5500 Offset: 0x21B1500 VA: 0x21B5500
  public void SetKillTimer(float time) { }
- // RVA: 0x21B9A48 Offset: 0x21B5A48 VA: 0x21B9A48
  public void CompleteTask(uint idx) { }
- // RVA: 0x21C1C28 Offset: 0x21BDC28 VA: 0x21C1C28
  public void RpcCompleteTask(uint idx) { }

## NetworkedPlayerInfo (was PlayerInfo)
public class NetworkedPlayerInfo : InnerNetObject
### Key Fields
- 0x35: public byte PlayerId
- 0x38: public int ClientId
- 0x50: public RoleTypes RoleType
- 0x64: public bool Disconnected
- 0x68: public RoleBehaviour Role
- 0x78: public bool IsDead
- 0x80: private PlayerControl _object
### Key Methods
- Offset 0x240FF74 / RVA 0x2413F74: public PlayerControl get_Object()
- Offset 0x2411BDC / RVA 0x2415BDC: public string get_PlayerName()
