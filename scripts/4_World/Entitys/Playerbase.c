modded class PlayerBase
{
	ref array< int > m_HeroesAndBandits_InZones = new ref array< int >; //For new Zones
	bool  m_HeroesAndBandits_Killed = false;
	
	int m_HeroesAndBandits_AffinityIndex = -1;
	float m_HeroesAndBandits_AffinityPoints = 0;
	bool m_HeroesAndBandits_DataLoaded = false;
	int m_HeroesAndBandits_LevelIndex = -1;
	
	override void Init()
	{
		super.Init();
		
		RegisterNetSyncVariableBool("m_HeroesAndBandits_Killed");
		
		RegisterNetSyncVariableInt("m_HeroesAndBandits_AffinityIndex");
		RegisterNetSyncVariableFloat("m_HeroesAndBandits_AffinityPoints");
		RegisterNetSyncVariableInt("m_HeroesAndBandits_LevelIndex");
		RegisterNetSyncVariableBool("m_HeroesAndBandits_DataLoaded");
		
	}
	
	override void OnPlayerLoaded(){
		super.OnPlayerLoaded();
		if (GetGame().IsServer() && GetIdentity() ){ 
			HeroesAndBanditsPlayer tempHABPlayer = GetHeroesAndBandits().GetPlayer(GetIdentity().GetPlainId());
			m_HeroesAndBandits_AffinityIndex = tempHABPlayer.getAffinityIndex();
			m_HeroesAndBandits_AffinityPoints = tempHABPlayer.getAffinityPoints(tempHABPlayer.getAffinityName());
			m_HeroesAndBandits_LevelIndex= tempHABPlayer.getLevelIndex();
			m_HeroesAndBandits_DataLoaded = true;
			habPrint("Player: " + GetIdentity().GetPlainId() + " Loaded with Affinty Index of " + m_HeroesAndBandits_AffinityIndex + " Points: " + m_HeroesAndBandits_AffinityPoints, "Debug");
			SetSynchDirty();
		}
	}
	
	void habLevelChange( int affinityIndex, float affinityPoints, int levelIndex){
		m_HeroesAndBandits_AffinityIndex = affinityIndex;
		m_HeroesAndBandits_AffinityPoints = affinityPoints;
	
		m_HeroesAndBandits_LevelIndex = levelIndex;
		SetSynchDirty();
	}
	
	void habCurrentAffinityPointUpdate(float affinityPoints){
		m_HeroesAndBandits_AffinityPoints = affinityPoints;
		SetSynchDirty();
	}
	
	int GetHeroesAndBanditsLevelIndex(){
		return m_HeroesAndBandits_LevelIndex;
	}
	
	bool isInZone(int zoneID, int index = 0)
	{
		if ( !m_HeroesAndBandits_InZones ){
			return false;
		} else if (m_HeroesAndBandits_InZones.Count() == 0) {
			return false;
		}
		return (m_HeroesAndBandits_InZones.Get(index) == zoneID);
	}
	
	void enteredZone(int zoneID, int index = 0)
	{
		int maxIndex =  m_HeroesAndBandits_InZones.Count() - 1;
		if ( index <= maxIndex ){
			leftZone(index);
		}
		habPrint("Player Entered Zone" + zoneID + " at index " + index, "Debug");
		m_HeroesAndBandits_InZones.Insert(zoneID);
		if ( GetHeroesAndBanditsSettings().DebugLogs ){
			m_HeroesAndBandits_InZones.Debug();
		}
	}
	
	void leftZone(int index){
		int maxIndex =  m_HeroesAndBandits_InZones.Count() - 1;
		if ( index == maxIndex ){
			habPrint("Removing Player from zone" + index, "Debug");
			m_HeroesAndBandits_InZones.Remove(index);
			if ( GetHeroesAndBanditsSettings().DebugLogs ){
				m_HeroesAndBandits_InZones.Debug();
			}
		} else if ( index < maxIndex ){
			habPrint("Removing Player from all subzones", "Debug");
			m_HeroesAndBandits_InZones.Remove(index);
			for ( int i = maxIndex; i >= index; i-- ){
				m_HeroesAndBandits_InZones.Remove(i);
			}
			if ( GetHeroesAndBanditsSettings().DebugLogs ){
				m_HeroesAndBandits_InZones.Debug();
			}
		} else {
			habPrint("Trying to leave zone not in" + index, "Exception");
			if ( GetHeroesAndBanditsSettings().DebugLogs ){
				m_HeroesAndBandits_InZones.Debug();
			}
		}
	}
	
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (!GetIdentity()){ return; } //If this isn't Player return
		m_HeroesAndBandits_Killed = true; //Pervent kills gettting counted twice with Explosions
		if (GetGame().IsServer() && GetIdentity()){
			bool killedByObject = false;
			string objectPlacerID = "";
			PlayerBase sourcePlayer;
			PlayerBase targetPlayer = this;
			string weaponName = "";
			if (killer.IsMan())	{
				sourcePlayer = PlayerBase.Cast(killer);
				weaponName = "#HAB_KILLFEED_FISTS";
			} else if (killer.IsWeapon()) {
				sourcePlayer = PlayerBase.Cast(EntityAI.Cast(killer).GetHierarchyParent());
				weaponName = "#HAB_KILLFEED_PRE "+ killer.GetDisplayName();
			} else if (killer.IsMeleeWeapon()) {
				sourcePlayer = PlayerBase.Cast(EntityAI.Cast(killer).GetHierarchyParent());
				weaponName = "#HAB_KILLFEED_PRE "+ killer.GetDisplayName();
			} else if (killer.IsTransport()){
				CarScript vehicle;
				if (Class.CastTo(vehicle, killer))
				{
					weaponName = "#HAB_KILLFEED_PRE "+ vehicle.GetDisplayName();
					if ( vehicle.CrewSize() > 0 )
					{
						PlayerBase driver = PlayerBase.Cast(vehicle.CrewMember( 0 ));
						if ( driver ){
							sourcePlayer = PlayerBase.Cast(driver);
						}
					}
				}
			} else if (killer.IsInherited(TrapBase)){
				TrapBase trap = TrapBase.Cast(killer);
				killedByObject = true;
				objectPlacerID = trap.habGetActivatedBy();
				weaponName =  "#HAB_KILLFEED_PRE " + trap.GetDisplayName();
				habPrint("Player " + targetPlayer.GetIdentity().GetPlainId() + " Killed by " + weaponName + " placed by " + objectPlacerID,"Debug");
			} 
			#ifdef EXPANSIONMOD
				else if ( killer.IsInherited(Expansion_C4_Explosion)){
					Expansion_C4_Explosion expansionExplosive = Expansion_C4_Explosion.Cast(killer);
					if ( expansionExplosive ){
						killedByObject = true;
						objectPlacerID = expansionExplosive.habGetActivatedBy();
						weaponName = "#HAB_KILLFEED_PRE " + "Home Made Explosive"; //TODO

						habPrint("Player " + targetPlayer.GetIdentity().GetPlainId() + " Killed by " + weaponName + " placed by " + objectPlacerID, "Debug");
					}
				}
			#endif
			else {
				if ( killer )
				{
					habPrint("Player " + targetPlayer.GetIdentity().GetPlainId() + " Killed by " + killer.GetType() ,"Debug");
				}
				return;
			}
			
			if (( sourcePlayer && targetPlayer ) || ( killedByObject && targetPlayer )) {//Make sure Players are valid
				string sourcePlayerID;
				if (killedByObject){
					sourcePlayerID = objectPlacerID;
				}else if ( sourcePlayer ) {
					sourcePlayerID = sourcePlayer.GetIdentity().GetPlainId();
				} else {
					//Something went wrong perventing a crash
					habPrint("Something went wrong with Player Killed" ,"Debug");
					return;
				}

				string targetPlayerID = targetPlayer.GetIdentity().GetPlainId();
				if (sourcePlayerID == targetPlayerID){ //Sucide
					if ( targetPlayer && !targetPlayer.IsInVehicle() ){//If not in Vehicle Crash
						GetHeroesAndBandits().NewPlayerAction(targetPlayerID, GetHeroesAndBandits().GetPlayerHeroOrBandit(targetPlayerID)+"Sucide");
						GetHeroesAndBandits().TriggerSucideFeed(targetPlayerID);
					}
				}else {
					GetHeroesAndBandits().NewPlayerAction(sourcePlayerID, GetHeroesAndBandits().GetPlayerHeroOrBandit(sourcePlayerID)+"Vs"+GetHeroesAndBandits().GetPlayerHeroOrBandit(targetPlayerID));
					GetHeroesAndBandits().TriggerKillFeed(sourcePlayerID, targetPlayerID, weaponName);
				}
			}
		}
	}
	
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{

		if ( damageType == DT_EXPLOSION && source && !this.IsAlive() && !m_HeroesAndBandits_Killed && GetGame().IsServer() && GetIdentity()) {
			m_HeroesAndBandits_Killed = true; //Pervent kills gettting counted twice with Explosions
			string sourcePlayerID;
			string targetPlayerID;
			string weaponName;
			if (source.IsInherited(Grenade_Base)){
				Grenade_Base grenade = Grenade_Base.Cast(source);
				string objectActivatedByID = grenade.habGetActivatedBy();
				weaponName =  "#HAB_KILLFEED_PRE " + grenade.GetType();
				habPrint("Player " + GetIdentity().GetPlainId() + " Killed by " + weaponName + " placed by " + objectActivatedByID,"Debug");
				sourcePlayerID = objectActivatedByID;
				targetPlayerID = GetIdentity().GetPlainId();
				if ( sourcePlayerID != "null" )
				{
					if (sourcePlayerID == targetPlayerID){ //Sucide
						GetHeroesAndBandits().NewPlayerAction(sourcePlayerID, GetHeroesAndBandits().GetPlayerHeroOrBandit(sourcePlayerID)+"Sucide");
						GetHeroesAndBandits().TriggerSucideFeed(sourcePlayerID);
					}else {
						GetHeroesAndBandits().NewPlayerAction(sourcePlayerID, GetHeroesAndBandits().GetPlayerHeroOrBandit(sourcePlayerID)+"Vs"+GetHeroesAndBandits().GetPlayerHeroOrBandit(targetPlayerID));
						GetHeroesAndBandits().TriggerKillFeed(sourcePlayerID, targetPlayerID, weaponName);
					}
				}
			} else {
				habPrint( "" + GetIdentity().GetPlainId() + " killed by Explosion with " + source.GetType(), "Debug");
			}
		} else  if ( damageType == DT_EXPLOSION && !source && !this.IsAlive() && GetGame().IsServer() && GetIdentity() ) {
			habPrint( "" + GetIdentity().GetPlainId() + " killed by Explosion with no source", "Debug");
		}
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	
	// making a safe way to grab the player withing the player class
	HeroesAndBanditsPlayer GetHaBPlayer()
	{
		if (GetGame().IsServer()){
			if (GetIdentity()){
				return GetHeroesAndBandits().GetPlayer(GetIdentity().GetPlainId());
			} 
		} else {
			if (g_HeroesAndBanditsPlayer){
				return g_HeroesAndBanditsPlayer;
			} else {
				habPrint("Player not defined on client","debug");
			}
		}
		return null;
	}

	
	override bool CanReceiveItemIntoCargo(EntityAI cargo)
	{
		if (!GetHeroesAndBanditsLevels() || !m_HeroesAndBandits_DataLoaded){
			return super.CanReceiveItemIntoCargo(cargo);
		}
		habAffinity tempAffinity = GetHeroesAndBanditsLevels().DefaultAffinity;
		if (m_HeroesAndBandits_AffinityIndex != -1){
			tempAffinity = GetHeroesAndBanditsLevels().Affinities.Get(m_HeroesAndBandits_AffinityIndex);
		}
		if (tempAffinity.checkItem(m_HeroesAndBandits_AffinityPoints, cargo.GetType(), "inventory"))
		{
			return super.CanReceiveItemIntoCargo(cargo);
		}
		return false;
	}
	
	override bool CanSwapItemInCargo(EntityAI child_entity, EntityAI new_entity)
	{
		if (!GetHeroesAndBanditsLevels() || !m_HeroesAndBandits_DataLoaded){
			return super.CanSwapItemInCargo(child_entity, new_entity);
		}
		habAffinity tempAffinity = GetHeroesAndBanditsLevels().DefaultAffinity;
		if (m_HeroesAndBandits_AffinityIndex != -1){
			tempAffinity = GetHeroesAndBanditsLevels().Affinities.Get(m_HeroesAndBandits_AffinityIndex);
		}
		if (tempAffinity.checkItem(m_HeroesAndBandits_AffinityPoints, new_entity.GetType(), "inventory"))
		{
			return super.CanSwapItemInCargo(child_entity, new_entity);
		}
		return false;
	}
	
	override bool CanReceiveItemIntoHands(EntityAI item_to_hands)
	{
		if (!GetHeroesAndBanditsLevels() || !m_HeroesAndBandits_DataLoaded){
			return super.CanReceiveItemIntoHands(item_to_hands);
		}
		habAffinity tempAffinity = GetHeroesAndBanditsLevels().DefaultAffinity;
		if (m_HeroesAndBandits_AffinityIndex != -1){
			tempAffinity = GetHeroesAndBanditsLevels().Affinities.Get(m_HeroesAndBandits_AffinityIndex);
		}
		if (tempAffinity.checkItem(m_HeroesAndBandits_AffinityPoints, item_to_hands.GetType(), "inhands"))
		{
			return super.CanReceiveItemIntoHands(item_to_hands);
		}
		return false;
	}
	
	override bool CanReleaseAttachment(EntityAI attachment)
	{
		if (!GetHeroesAndBanditsLevels() || !m_HeroesAndBandits_DataLoaded){
			return super.CanReleaseAttachment(attachment);
		}
		habAffinity tempAffinity = GetHeroesAndBanditsLevels().DefaultAffinity;
		if (m_HeroesAndBandits_AffinityIndex != -1){
			tempAffinity = GetHeroesAndBanditsLevels().Affinities.Get(m_HeroesAndBandits_AffinityIndex);
		}
		ClothingBase mask = ClothingBase.Cast(GetInventory().FindAttachment(InventorySlots.MASK));
		if (mask){
			if (attachment == mask && !GetHeroesAndBanditsSettings().BanditsCanRemoveMask && tempAffinity.Name == "bandit"){
				return false;
			}
		}
		return super.CanReleaseAttachment(attachment);
	}
	
	
	
	override bool CanReceiveAttachment(EntityAI attachment, int slotId)
	{
		if (!GetHeroesAndBanditsLevels() || !m_HeroesAndBandits_DataLoaded){
			return super.CanReceiveAttachment(attachment, slotId);
		}
		habAffinity tempAffinity = GetHeroesAndBanditsLevels().DefaultAffinity;
		if (m_HeroesAndBandits_AffinityIndex != -1){
			tempAffinity = GetHeroesAndBanditsLevels().Affinities.Get(m_HeroesAndBandits_AffinityIndex);
		}
		if (tempAffinity.checkItem(m_HeroesAndBandits_AffinityPoints, attachment.GetType(), "attach"))
		{
			return super.CanReceiveAttachment(attachment, slotId);
		}
		return false;
			
	}

}
