#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Vbap.hpp"

namespace al{

bool SpeakerTriple::loadVectors(const std::vector<Speaker>& spkrs){
	bool hasInverse;

	s1Vec = spkrs[s1].vec();
	s2Vec = spkrs[s2].vec();

	s1Chan = spkrs[s1].deviceChannel;
	s2Chan = spkrs[s2].deviceChannel;

	if(s3!=-1){ // 3d Speaker layout
		s3Vec = spkrs[s3].vec();
		s3Chan = spkrs[s3].deviceChannel;
		mat.set(s1Vec[0],s1Vec[1],s1Vec[2],
		        s2Vec[0],s2Vec[1],s2Vec[2],
		        s3Vec[0],s3Vec[1],s3Vec[2]
		        );
		hasInverse = invert(mat);

	}else{  // 2d Speaker layout
		Mat<2,double> tempMat;
		tempMat.set(s1Vec[0],s1Vec[1],
		        s2Vec[0],s2Vec[1]
		        );
		hasInverse = invert(tempMat);

		//insert inverted matrix into 3x3 matrix
		mat.set(tempMat(0,0),tempMat(0,1),s1Vec[2],
		        tempMat(1,0),tempMat(1,1),s2Vec[2],
		        s3Vec[0],s3Vec[1],s3Vec[2]
		        );
	}
	vec[0]=s1Vec;
	vec[1]=s2Vec;
	vec[2]=s3Vec;

	//For finding crossings
	speakerIdx[0] = s1;
	speakerIdx[1] = s2;
	speakerIdx[2] = s3;

	speakerChan[0] = s1Chan;
	speakerChan[1] = s2Chan;
	speakerChan[2] = s2Chan;

	return hasInverse;
}



Vbap::Vbap(const SpeakerLayout &sl, bool is3D)
    :	Spatializer(sl), mIs3D(is3D)
{
	//Check if 3D...
	if(mIs3D){
		printf("Finding triplets\n");
		findSpeakerTriplets(mSpeakers);
	}
	else{
		printf("Finding pairs\n");
		findSpeakerPairs(mSpeakers);
	}

	if (mTriplets.size() == 0 ){
		printf("No SpeakerSets found. Check mode setting or speaker layout.\n");
		throw -1;
	}

}

void Vbap::addTriple(const SpeakerTriple& st) {
	mTriplets.push_back(st);
}

Vec3d Vbap::computeGains(const Vec3d& vecA, const SpeakerTriple& speak) {
	const Mat3d& mat = speak.mat;
	unsigned dimensions = mIs3D ? 3 : 2;
	Vec3d vec(0., 0., 0.);

	// For each node, insert speaker coordinates in matrix
	for (unsigned i = 0; i < dimensions; i++){
		for (unsigned j = 0; j < dimensions; j++){
			vec[i] += vecA[j] * mat(j,i);
		}
	}
	return vec;
}


// 2D VBAP, find pairs of speakers.
void Vbap::findSpeakerPairs(const std::vector<Speaker>& spkrs){

	unsigned numSpeakers = spkrs.size();
	unsigned j, index;
	unsigned speakerMapping[numSpeakers]; // To map unordered speakers into an ordered set.
	float speakerAngles[numSpeakers];
	float indexAngle;

	// Build a map to the speaker, that points to speaker indexes.
	for (unsigned i = 0; i < numSpeakers; i++) {
		speakerAngles[i] = spkrs[i].azimuth;

		// speakerMapping[i] = spkrs[i].deviceChannel;
		speakerMapping[i] = i;
	}

	// Sort speakers into the map
	for (unsigned i = 1; i < numSpeakers; i++) {
		// Only sort speakers that have elevation == 0. Ignore all other.
		if (spkrs[i].elevation == 0) {

			indexAngle = speakerAngles[i];
			index = speakerMapping[i];
			j = i;

			while ((j > 0) && (speakerAngles[j-1] > indexAngle)) {
				speakerAngles[j] = speakerAngles[j-1];
				speakerMapping[j] = speakerMapping[j-1];
				j = j - 1;

			}
			speakerAngles[j] = indexAngle;
			speakerMapping[j] = index;
		}
	}

	// Add speaker-pairs
	for (unsigned i = 1; i < numSpeakers; i++){
		SpeakerTriple triple;
		triple.s1 = speakerMapping[i-1];
		triple.s2 = speakerMapping[i];
		triple.s3 = -1;
		triple.loadVectors(spkrs);
		addTriple(triple);
	}

	// Add the last speaker-pair
	SpeakerTriple triple;
	triple.s1 = speakerMapping[numSpeakers-1];
	triple.s2 = speakerMapping[0];
	triple.s3 = -1;
	triple.loadVectors(spkrs);
	addTriple(triple);
}

bool Vbap::isCrossing(Vec3d c, Vec3d li, Vec3d lj, Vec3d ln, Vec3d lm){
	double thresh = 0.0001;
	double a1 = angle(c,li)+angle(c,lj);
	double a2 = angle(li,lj);
	double a3 = angle(c,ln)+angle(c,lm);
	double a4 = angle(ln,lm);
	return (fabs(a1-a2)< thresh) && (fabs(a3-a4)< thresh);
}

void Vbap::findSpeakerTriplets(const std::vector<Speaker>& spkrs){
	std::list<SpeakerTriple> triplets;

	unsigned numSpeakers = spkrs.size();
	int numSpeakersSigned = (int)numSpeakers;

	// form all possible triples
	for (unsigned i = 0; i < numSpeakers; i++){
		for (unsigned j = i+1; j < numSpeakers; j++){
			for (unsigned k = j+1; k < numSpeakers; k++){
				SpeakerTriple triplet;
				triplet.s1=i;
				triplet.s2=j;
				triplet.s3=k;

				//Only add triplet if its matrix is invertable
				if(triplet.loadVectors(spkrs)){
				}
				triplets.push_back(triplet);
			}
		}
	}
	printf("Speaker-count=%d, Initial triplet-count=%d\n",numSpeakers,(unsigned)triplets.size());

	//RemoveTriangles that have equal elevation
	int equalElevCounter = 0;
	for(std::list<SpeakerTriple>::iterator it = triplets.begin(); it != triplets.end();){
		SpeakerTriple trip = (*it);
		double a = trip.s1Vec[2];
		double b = trip.s2Vec[2];
		double c = trip.s3Vec[2];

		if((a==b) && (a == c)){
			it =  triplets.erase(it);
			equalElevCounter++;

		}else{
			++it;
		}
	}
	printf("Tris removed because equal elev %i\n",equalElevCounter);

	//Remove Sides with equal elevation that have a speaker inbetween them
	std::list<SpeakerTriple>::iterator itA = triplets.begin();
	int equalElevBtwCounter = 0;
	while(itA != triplets.end()){
		bool breakOuter = false;
		SpeakerTriple trip = (*itA);
		//int numSpeaks = 3;

		for(int i = 0 ;i < 3;i++){

			int spkIdx1 = i%3;
			int spkIdx2 = (i+1)%3;

			if(trip.vec[spkIdx1].z != trip.vec[spkIdx2].z){
				continue;
			}

			//Set z to 0
			Vec3d vec1(trip.vec[spkIdx1].x,trip.vec[spkIdx1].y,0.f);
			Vec3d vec2(trip.vec[spkIdx2].x,trip.vec[spkIdx2].y,0.f);

			for(Speaker s: spkrs){
				if(s.vec().z != trip.vec[spkIdx1].z){
					continue;
				}

				if( trip.speakerChan[spkIdx1] == (int)s.deviceChannel || trip.speakerChan[spkIdx2] == (int)s.deviceChannel){
					continue;
				}

				Vec3d spkVec(s.vec().x,s.vec().y,0.f);

				if( angle(vec1,vec2) > angle(vec1,spkVec) &&  angle(vec1,vec2) > angle(vec2,spkVec)){
					itA = triplets.erase(itA);
					breakOuter = true;
					equalElevBtwCounter++;
					break;
				}
			}
		}
		if(breakOuter){
			continue;
		}
		++itA;
	}
	printf("Tris removed because equal elev with spk btw %i\n",equalElevBtwCounter);


	// remove too narrow triples
	std::list<SpeakerTriple>::iterator nit = triplets.begin();
	int narrowCounter=0;
	while(nit != triplets.end()){
		SpeakerTriple trip = (*nit);

		//Scalar triple product (a x b) dot c
		Vec3d a = trip.s1Vec.normalized();
		Vec3d b = trip.s2Vec.normalized();
		Vec3d c = trip.s3Vec.normalized();
		Vec3d xprod = cross(a,b);

		float volume = fabs(xprod.dot(c));

		float length = fabs(angle(a , b) ) + fabs(angle(a , c) ) + fabs(angle(b , c) );
		float ratio;

		if (length > MIN_LENGTH){
			ratio =  volume / length;
		}else{
			ratio = 0.0;
		}

		if (ratio < MIN_VOLUME_TO_LENGTH_RATIO) {
			nit =triplets.erase(nit);
			narrowCounter++;
		}else{
			++nit;
		}
	}
	printf("Triangles removed because too narrow %i\n",narrowCounter);


	//Remove triplet with longest side if crossing
	int crossCounter = -1;
	while(crossCounter != 0){
		crossCounter=0;

		std::list<SpeakerTriple>::iterator it = triplets.begin();

		while(it != triplets.end()){
			bool breakOuter = false;

			SpeakerTriple trip = (*it);

			std::list<SpeakerTriple>::iterator it2 = triplets.begin();
			while(it2 != triplets.end()){
				bool breakInner = false;
				SpeakerTriple trip2 = (*it2);

				if((trip.s1 == trip2.s1) && (trip.s2 == trip2.s2) && (trip.s3== trip2.s3) ){
					++it2;
					continue;
				}

				for(int i = 0 ;i < 3;i++){
					for(int j = 0; j < 3; j++){

						int a = trip.speakerIdx[i%3];
						int b = trip.speakerIdx[(i+1)%3];
						int c = trip2.speakerIdx[j%3];
						int d = trip2.speakerIdx[(j+1)%3];

						//Check to see if the pairs have a speaker in common
						if((a==c)||(a==d) || (b==c)||(b==d)){
							continue;
						}

						Vec3d li = trip.vec[i%3].normalized();
						Vec3d lj = trip.vec[(i+1)%3].normalized();
						Vec3d ln = trip2.vec[j%3].normalized();
						Vec3d lm = trip2.vec[(j+1)%3].normalized();

						Vec3d cr = cross(cross(li, lj),cross(ln,lm));

						double lt = dist(trip.vec[i%3], trip.vec[(i+1)%3]);
						double lt2 =dist(trip2.vec[j%3],trip2.vec[(j+1)%3]);

						if (isCrossing(cr,li,lj,ln,lm) || isCrossing(-cr,li,lj,ln,lm)) {

							//TODO: how to handle the case where lt == lt2
							if(lt > lt2){
								it = triplets.erase(it);
								breakOuter = true;
								crossCounter++;
							}else if (lt < lt2){
								it2 =triplets.erase(it2);
								breakInner = true;
								crossCounter++;
							}else{

							}
						}
						if(breakInner || breakOuter){break;}
					}
					if(breakInner || breakOuter){break;}
				}
				if(breakInner || breakOuter){break;}
				++it2;
			}
			if(breakOuter){continue;}
			++it;
		}
		printf("Triangles removed because of crossing %i\n",crossCounter);
	}



	//Remove triplet if contains another speaker
	double thresh = 0.f;
	int spkInTri = 0;
	for(int i = 0; i < numSpeakersSigned; ++i){
		Speaker s = spkrs[i];
		Vec3d vec =s.vec().normalized();
		int devChan = s.deviceChannel;

		std::list<SpeakerTriple>::iterator itg = triplets.begin();

		while(itg != triplets.end()){
			SpeakerTriple trip2 = (*itg);
			if ((devChan == trip2.s1Chan) || (devChan == trip2.s2Chan) || (devChan == trip2.s3Chan) ){
				++itg;
				continue;
			}

			Vec3d  gains = computeGains(vec, trip2);

			//if ((gains[0] > 0) && (gains[1] > 0) && (gains[2] > 0) ){
			if ((gains[0] > thresh) && (gains[1] > thresh) && (gains[2] > thresh) ){
				itg = triplets.erase(itg);
				spkInTri++;
			}else{
				++itg;
			}
		}
	}
	printf("Tris removed because spk inside triangle %i\n",spkInTri);

	std::list<SpeakerTriple>::iterator it3 = triplets.begin();
	while(it3 != triplets.end()) {
		addTriple(*it3);
		++it3;
	}
}

void Vbap::makePhantomChannel(int channelIndex, std::vector<int> assignedOutputs)
{
	mPhantomChannels[channelIndex] = assignedOutputs;
}

void Vbap::compile(Listener& listener){
	this->mListener = &listener;
}

void Vbap::renderBuffer(AudioIOData &io, const Pose &listeningPose, const float *samples, const int &numFrames)
{
	// FIMXE AC use cached index
//	unsigned currentTripletIndex = src.cachedIndex();
	unsigned currentTripletIndex = 0;
	// unsigned currentTripletIndex = mCachedTripletIndex; // Cached source placement, so it starts searching from there.

	Vec3d vec = listeningPose.vec();

	//Rotate vector according to listener-rotation
	Quatd srcRot = listeningPose.quat();
	vec = srcRot.rotate(vec).get<0,2,1>();

	//Silent by default
	Vec3d gains;

	// Search thru the triplets array in search of a match for the source position.
	for (unsigned count = 0; count < mTriplets.size(); ++count) {
		gains = computeGains(vec, mTriplets[currentTripletIndex]);
		if ((gains[0] >= 0) && (gains[1] >= 0) && (!mIs3D || (gains[2] >= 0)) ){
			gains.normalize();

			const SpeakerTriple triple = mTriplets[currentTripletIndex];

			// Check if any of the triplets are phantom channels and
			// reassign signal
			auto it1 = mPhantomChannels.find(triple.s1Chan);
			auto it2 = mPhantomChannels.find(triple.s2Chan);
			auto it3 = mPhantomChannels.find(triple.s3Chan);

			for(int i = 0; i < numFrames; ++i){
				if (it1 != mPhantomChannels.end()) { // vertex 1 is phantom
					float splitGain = gains[0] /mPhantomChannels.size();
					float splitGainSQ = splitGain * splitGain;
					for(auto const &element : it1->second) { // iterate across all assigned speakers
						io.bufferOut().at(i, element) += samples[i]*splitGainSQ;
					}
				} else {
					io.bufferOut().at(i, triple.s1Chan) += samples[i]*gains[0];
				}
				if (it2 != mPhantomChannels.end()) { // vertex 2 is phantom
					float splitGain = gains[1] /mPhantomChannels.size();
					float splitGainSQ = splitGain * splitGain;
					for(auto const &element : it2->second) {
						io.bufferOut().at(i, element) += samples[i]*splitGainSQ;
					}
				} else {
					io.bufferOut().at(i, triple.s2Chan)  += samples[i]*gains[1];
				}
				if(mIs3D){
					if (it3 != mPhantomChannels.end()) {
						float splitGain = gains[2] /mPhantomChannels.size();
						float splitGainSQ = splitGain * splitGain;
						for(auto const &element : it3->second) {
							io.bufferOut().at(i, element) += samples[i]*splitGainSQ;
						}
					} else {
						io.bufferOut().at(i, triple.s3Chan)  += samples[i]*gains[2];
					}
				}

			}
			break;
		}

		++currentTripletIndex;
		if (currentTripletIndex >= mTriplets.size()){
			currentTripletIndex = 0;
		}
	}
	// FIXME AC store cached index
//	src.cachedIndex(currentTripletIndex);
	//mCachedTripletIndex = currentTripletIndex; // Store the new index
}

void Vbap::renderSample(AudioIOData &io, const Pose &listeningPose, const float &sample, const int &frameIndex)
{
	// FIXME AC use cached index
//	unsigned currentTripletIndex = src.cachedIndex();
	//unsigned currentTripletIndex = mCachedTripletIndex; // Cached source placement, so it starts searching from there.
	unsigned currentTripletIndex = 0;
	Vec3d vec = listeningPose.vec();

	//Rotate vector according to listener-rotation
	Quatd srcRot = listeningPose.quat();
	vec = srcRot.rotate(vec).normalize();

	// now transform to audio positions
	vec = vec.get<0,2,1>();
	//Silent by default
	Vec3d gains;
//	Vec3d gainsTemp;

	// Search thru the triplets array in search of a match for the source position.
	for (unsigned count = 0; count < mTriplets.size(); ++count) {
		gains = computeGains(vec, mTriplets[currentTripletIndex]);
		if ((gains[0] >= 0) && (gains[1] >= 0) && (!mIs3D || (gains[2] >= 0)) ){
			gains.normalize();
			break;
		}

		++currentTripletIndex;
		if (currentTripletIndex >= mTriplets.size()){
			currentTripletIndex = 0;
		}
	}

	const auto triple = mTriplets[currentTripletIndex];

	//mCachedTripletIndex = currentTripletIndex; // Store the new index
	// FIXME stored cached index
//	src.cachedIndex(currentTripletIndex);

	// Check if any of the triplets are phantom channels and
	// reassign signal
	auto it1 = mPhantomChannels.find(triple.s1Chan);
	auto it2 = mPhantomChannels.find(triple.s2Chan);
	auto it3 = mPhantomChannels.find(triple.s3Chan);

	auto& s1 = io.bufferOut().at(frameIndex, triple.s1Chan);
	auto& s2 = io.bufferOut().at(frameIndex, triple.s2Chan);
	auto& s3 = io.bufferOut().at(frameIndex, triple.s3Chan);

	if (it1 != mPhantomChannels.end()) { // vertex 1 is phantom
		float splitGain = gains[0]* gains[0] /2.0;
		s1 = 0.0;
		s2 += sample*splitGain;
		s3 += sample*splitGain;
	} else {
		s1 += sample * gains[0];
	}
	if (it2 != mPhantomChannels.end()) { // vertex 2 is phantom
		float splitGain = gains[1]* gains[1] /2.0;
		s1 += sample*splitGain;
		s2 = 0.0;
		s3 += sample*splitGain;
	} else {
		s2 += sample * gains[1];
	}
	if(mIs3D){
		if (it2 != mPhantomChannels.end()) {
			float splitGain = gains[2]* gains[2] /2.0;
			s1 += sample*splitGain;
			s2 += sample*splitGain;
			s3 = 0.0;
		} else {
			s3 += sample * gains[2];
		}
	}
}

////Per buffer
//void Vbap::perform(AudioIOData& io,SoundSource& src,Vec3d& relpos,const int& numFrames,float *samples){

//}

////per sample
//void Vbap::perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){

//}

void Vbap::print() {
	printf("Number of Triplets: %d\n", (int) mTriplets.size());
	for (unsigned i = 0; i < mTriplets.size(); i++) {
		printf("Triple #%d: %d,%d,%d \n",i,mTriplets[i].s1Chan,mTriplets[i].s2Chan,mTriplets[i].s3Chan);
	}
}

void Vbap::makeTriple(int s1, int s2, int s3)
{
	SpeakerTriple triple;
	triple.s1 = s1;
	triple.s2 = s2;
	triple.s3 = s3;
	triple.loadVectors(mSpeakers);
	addTriple(triple);
}
std::vector<SpeakerTriple> Vbap::triplets() const
{
	return mTriplets;
}


} // al::
